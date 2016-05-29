/*
 * Author: Shiran Ben-Melech <shiran.ben-melech@intel.com>
 * Copyright (c) 2016 Intel Corporation.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "littleb.h"

sd_event *event = NULL;

const char*
_convert_device_path_to_address(const char *address)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        int i;
        const char *prefix = "dev_";

        // find the address in the object string - after "dev_"
        const char  *start_of_address = strstr(address, prefix) + strlen(prefix)  *sizeof(char);

        char *new_address = strdup(start_of_address);

        if(new_address == NULL) {
                syslog(LOG_ERR, "%s: Error copying address to new_addresss", __FUNCTION__);
                return NULL;
        }

        for(i = 0; i < strlen(new_address); i++) {
                if(new_address[i] == '_') {
                        new_address[i] = ':';
                }
        }

        return (const char*) new_address;
}

bool
_is_bus_connected(lb_context *lb_ctx)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        if(lb_ctx->bus == NULL) {
                return false;
        }
        else {
                return (sd_bus_is_open(lb_ctx->bus)) ? true : false;
        }
}

lb_result_t
_get_root_objects(lb_context *lb_ctx, const char **objects)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        int r = 0, i = 0;
        const char *device_path;
        sd_bus_error error = SD_BUS_ERROR_NULL;
        sd_bus_message *m = NULL;

        if(!_is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               BLUEZ_DEST,
                               "/",
                               "org.freedesktop.DBus.ObjectManager",
                               "GetManagedObjects",
                               &error,
                               &m,
                               NULL);
        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_call_method GetManagedObjects failed with error: %s", __FUNCTION__, error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        /* Parse the response message */
        //"a{oa{sa{sv}}}"
        r = sd_bus_message_enter_container(m, 'a', "{oa{sa{sv}}}");
        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_message_enter_container {oa{sa{sv}}} failed with error: %s", __FUNCTION__, strerror(-r));
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        while((r = sd_bus_message_enter_container(m, 'e', "oa{sa{sv}}")) > 0) {
                r = sd_bus_message_read_basic(m, 'o', &device_path);
                if(r < 0) {
                        syslog(LOG_ERR, "%s: sd_bus_message_read_basic failed with error: %s", __FUNCTION__, strerror(-r));
                                sd_bus_error_free(&error);
        sd_bus_message_unref(m);;
                }
                else {
                        char *restrict new_device_path = (char *restrict )malloc(strlen(device_path) + 1);
                        if(new_device_path == NULL) {
                                syslog(LOG_ERR, "%s: Error allocating memory for object name", __FUNCTION__);
                                sd_bus_error_free(&error);
                                sd_bus_message_unref(m);
                                return -LB_ERROR_UNSPECIFIED;
                        }
                        objects[i] = strcpy(new_device_path, device_path);
                        i++;
                }

                r = sd_bus_message_skip(m, "a{sa{sv}}");
                if(r < 0) {
                        syslog(LOG_ERR, "%s: sd_bus_message_skip failed with error: %s", __FUNCTION__, strerror(-r));
                        sd_bus_error_free(&error);
                        sd_bus_message_unref(m);
                        return -LB_ERROR_UNSPECIFIED;
                }

                r = sd_bus_message_exit_container(m);
                if(r < 0) {
                        syslog(LOG_ERR, "%s: sd_bus_message_exit_container oa{sa{sv}} failed with error: %s", __FUNCTION__, strerror(-r));
                        sd_bus_error_free(&error);
                        sd_bus_message_unref(m);
                        return -LB_ERROR_UNSPECIFIED;
                }
        }

        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_message_enter_container oa{sa{sv}} failed with error: %s", __FUNCTION__, strerror(-r));
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_message_exit_container(m);
        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_message_exit_container {oa{sa{sv}}} failed with error: %s", __FUNCTION__, strerror(-r));
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return LB_SUCCESS;
}

const char*
_get_device_name(lb_context *lb_ctx, const char *device_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        sd_bus_error error = SD_BUS_ERROR_NULL;
        int r;
        char *name;

        r = sd_bus_get_property_string(lb_ctx->bus, BLUEZ_DEST, device_path, BLUEZ_DEVICE, "Name", &error, &name);
        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_get_property_string Name on device %s failed with error: %s", __FUNCTION__, device_path, error.message);
                sd_bus_error_free(&error);
                return NULL;
        }

        return (const char *) name;
}

const char*
_get_device_address(lb_context *lb_ctx, const char *device_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        sd_bus_error error = SD_BUS_ERROR_NULL;
        int r;
        char *address;

        r = sd_bus_get_property_string(lb_ctx->bus, BLUEZ_DEST, device_path, BLUEZ_DEVICE, "Address", &error, &address);
        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_get_property_string Address on device %s failed with error: %s", __FUNCTION__, device_path, error.message);
                sd_bus_error_free(&error);
                return NULL;
        }

        return (const char *) address;
}

const char*
_get_service_uuid(lb_context *lb_ctx, const char *service_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        sd_bus_error error = SD_BUS_ERROR_NULL;
        int r;
        char *name;

        r = sd_bus_get_property_string(lb_ctx->bus, BLUEZ_DEST, service_path, "org.bluez.GattService1", "UUID", &error, &name);
        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_get_property_string UUID on service %s failed with error: %s", __FUNCTION__,service_path, error.message);
                sd_bus_error_free(&error);
                return NULL;
        }

        return (const char *) name;
}

const char*
_get_characteristic_uuid(lb_context *lb_ctx, const char *characteristic_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        sd_bus_error error = SD_BUS_ERROR_NULL;
        int r;
        char *name;

        r = sd_bus_get_property_string(lb_ctx->bus, BLUEZ_DEST, characteristic_path, BLUEZ_GATT_CHARACTERISTICS, "UUID", &error, &name);
        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_get_property_string UUID on characteristic: %s failed with error: %s", __FUNCTION__, characteristic_path, error.message);
                sd_bus_error_free(&error);
                return NULL;
        }

        return (const char *) name;
}

bool
_is_string_in_device_introspection(lb_context *lb_ctx, const char *device_path, const char *str)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        sd_bus_error error = SD_BUS_ERROR_NULL;
        sd_bus_message *m = NULL;

        int r;
        const char *introspect_xml;

        if(!_is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               BLUEZ_DEST,
                               device_path,
                               "org.freedesktop.DBus.Introspectable",
                               "Introspect",
                               &error,
                               &m,
                               NULL);
        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_call_method Introspect on device %s failed with error: %s", __FUNCTION__, device_path, error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);;
                return false;
        }

        r = sd_bus_message_read_basic(m, 's', &introspect_xml);
        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_message_read_basic failed with error: %s", __FUNCTION__, strerror(-r));
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return false;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);

        return (strstr(introspect_xml, str) != NULL) ? true : false;
}

bool
_is_bl_device(lb_context *lb_ctx, const char *device_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        return _is_string_in_device_introspection(lb_ctx, device_path, BLUEZ_DEVICE);
}

bool
_is_ble_device(lb_context *lb_ctx, const char *device_path)
{

        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        const char* *objects;
        int i = 0, r = 0;

        if (!_is_bl_device(lb_ctx, device_path)) {
                syslog(LOG_ERR, "%s: not a bl device", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }

        objects = (const char **)calloc(MAX_OBJECTS, MAX_OBJECTS  *sizeof(const char *));
        if(objects == NULL) {
                syslog(LOG_ERR, "%s: Error allocating memory for objects array", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = _get_root_objects(lb_ctx, objects);

        if (r < 0) {
              syslog(LOG_ERR, "%s: Error getting root objects", __FUNCTION__);
        //      free(objects);
              return false;
        }

        while(objects[i] != NULL) {
                if( (strstr(objects[i], device_path) != NULL) && (strstr(objects[i], "service") != NULL)) {
                        return true;
                }
                i++;
        }

        free(objects);
        return false;
}

bool
_is_ble_service(lb_context *lb_ctx, const char *service_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        return _is_string_in_device_introspection(lb_ctx, service_path, BLUEZ_GATT_SERVICE);
}

bool
_is_ble_characteristic(lb_context *lb_ctx, const char *service_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        return _is_string_in_device_introspection(lb_ctx, service_path, BLUEZ_GATT_CHARACTERISTICS);
}

bool
_is_service_primary(lb_context *lb_ctx, const char *service_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        sd_bus_error error = SD_BUS_ERROR_NULL;
        int r;
        bool primary;

        r = sd_bus_get_property_trivial(lb_ctx->bus, BLUEZ_DEST, service_path, "org.bluez.GattService1", "Primary", &error, 'b', &primary);
        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_get_property_trivial Primary on service %s failed with error: %s", __FUNCTION__, service_path, error.message);
                sd_bus_error_free(&error);
                return NULL;
        }

        return primary;
}

bool
_is_device_paired(lb_context *lb_ctx, const char *device_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        sd_bus_error error = SD_BUS_ERROR_NULL;
        int r;
        bool paired;

        r = sd_bus_get_property_trivial(lb_ctx->bus, BLUEZ_DEST, device_path, BLUEZ_DEVICE, "Paired", &error, 'b', &paired);
        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_get_property_trivial Paired on device %s failed with error: %s", __FUNCTION__, device_path, error.message);
                sd_bus_error_free(&error);
                return NULL;
        }

        return paired;
}

lb_result_t
_add_new_characteristic(lb_context *lb_ctx, ble_service *service, const char *characteristic_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);

        int current_index = service->characteristics_size;
        if(service->characteristics_size == 0 || service->characteristics == NULL) {
                service->characteristics = (ble_char **)malloc(sizeof(ble_char*));
                service->characteristics_size++;
        }
        else {
                service->characteristics_size++;
                service->characteristics = realloc(service->characteristics, (service->characteristics_size) * sizeof(ble_char*));
                if(service->characteristics == NULL) {
                        syslog(LOG_ERR, "%s: Error reallocating memory for characteristics", __FUNCTION__);
                        return -LB_ERROR_UNSPECIFIED;
                }
        }

        ble_char *new_characteristic= (ble_char *)malloc(sizeof(ble_char));

        new_characteristic->char_path = characteristic_path;

        const char *uuid = _get_characteristic_uuid(lb_ctx, characteristic_path);
        if(uuid == NULL) {
                syslog(LOG_ERR, "%s: Error couldn't find characteristic uuid", __FUNCTION__);
                new_characteristic->uuid = "null";
        }
        else {
                new_characteristic->uuid = uuid;
        }

        service->characteristics[current_index] = new_characteristic;

        return LB_SUCCESS;
}

lb_result_t
_add_new_service(lb_context *lb_ctx, bl_device* bl_dev, const char *service_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);

        if (!_is_ble_device(lb_ctx,bl_dev->device_path)) {
                syslog(LOG_ERR, "%s: not a ble device", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }

        int current_index = bl_dev->services_size;
        if(bl_dev->services_size == 0 || bl_dev->services == NULL) {
                bl_dev->services = (ble_service**)malloc(sizeof(ble_service*));
                bl_dev->services_size++;
        }
        else {
                bl_dev->services_size++;
                bl_dev->services = realloc(bl_dev->services, (bl_dev->services_size) * sizeof(ble_service*));
                if(bl_dev->services == NULL) {
                        syslog(LOG_ERR, "%s: Error reallocating memory for services", __FUNCTION__);
                        return -LB_ERROR_UNSPECIFIED;
                }
        }

        ble_service *new_service = (ble_service*)malloc(sizeof(ble_service));

        new_service->service_path = service_path;

        const char *uuid = _get_service_uuid(lb_ctx, service_path);
        if(uuid == NULL) {
                syslog(LOG_ERR, "%s: Error couldn't find service uuid", __FUNCTION__);
                new_service->uuid = "null";
        }
        else {
                new_service->uuid = uuid;
        }

        new_service->primary = _is_service_primary(lb_ctx, service_path);

        new_service->characteristics = NULL;
        new_service->characteristics_size = 0;

        bl_dev->services[current_index] = new_service;

        return LB_SUCCESS;
}

lb_result_t
_add_new_device(lb_context *lb_ctx, const char *device_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);

        int current_index = lb_ctx->devices_size;
        if(lb_ctx->devices_size == 0 || lb_ctx->devices == NULL) {
                lb_ctx->devices = (bl_device**)malloc(sizeof(bl_device*));
                lb_ctx->devices_size++;
        }
        else {
                lb_ctx->devices_size++;
                lb_ctx->devices = realloc(lb_ctx->devices, (lb_ctx->devices_size)  *sizeof(bl_device*));
                if(lb_ctx->devices == NULL) {
                        syslog(LOG_ERR, "%s: Error reallocating memory for devices", __FUNCTION__);
                        return -LB_ERROR_UNSPECIFIED;
                }
        }

        bl_device *new_device = (bl_device*)malloc(sizeof(bl_device));

        new_device->device_path = device_path;
        const char *name = _get_device_name(lb_ctx, device_path);
        if(name == NULL) {
                syslog(LOG_ERR, "%s: Error couldn't find device name", __FUNCTION__);
                new_device->name = "null";
        }
        else {
                new_device->name = name;
        }
        const char *address = _get_device_address(lb_ctx, device_path);
        if(address == NULL) {
                syslog(LOG_ERR, "%s: Error couldn't find device address", __FUNCTION__);
                new_device->address = "null";
        }
        else {
                new_device->address = address;
        }

        new_device->services = NULL;
        new_device->services_size = 0;

        //new_device->address = convert_device_path_to_address(device_path);
        lb_ctx->devices[current_index] = new_device;

        return LB_SUCCESS;
}

lb_result_t
_scan_devices(lb_context *lb_ctx, int seconds)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        int r = 0;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(!_is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_call_method(lb_ctx->bus, BLUEZ_DEST, "/org/bluez/hci0", "org.bluez.Adapter1", "StartDiscovery", &error, &m,
        NULL);
        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_call_method StartDiscovery failed with error: %s", __FUNCTION__, error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        sleep(seconds);

        r = sd_bus_call_method(lb_ctx->bus, BLUEZ_DEST, "/org/bluez/hci0", "org.bluez.Adapter1", "StopDiscovery", &error, &m,
        NULL);

        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_call_method StopDiscovery failed with error: %s", __FUNCTION__, error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return LB_SUCCESS;
}

lb_result_t
_open_system_bus(lb_context *lb_ctx)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;

        /* Connect to the system bus */
        r = sd_bus_open_system(&(lb_ctx->bus));
        if(r < 0) {
                syslog(LOG_ERR, "%s: Failed to connect to system bus: %s", __FUNCTION__, strerror(-r));
        }

        return r;
}

lb_result_t
_close_system_bus(lb_context *lb_ctx)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        sd_bus_unref(lb_ctx->bus);
        if(lb_ctx->devices) {
                free(lb_ctx->devices);
                lb_ctx->devices_size = 0;
        }
        return LB_SUCCESS;
}

lb_result_t
lb_init()
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        r = sd_event_default(&event);
        if (r < 0) {
                syslog(LOG_ERR, "%s: Failed to allocate event loop", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_event_set_watchdog(event, true);
        if (r < 0) {
                syslog(LOG_ERR, "%s: Failed to set watchdog", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }
        return LB_SUCCESS;
}

lb_result_t
lb_destroy()
{
        return LB_SUCCESS;
}

lb_result_t
lb_context_new(lb_context **lb_ctx)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r = 0;
        lb_context *new_context;

        if(*lb_ctx != NULL) {
                syslog(LOG_ERR, "%s: lb_context is not empty!", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }

        new_context = (lb_context*)malloc(sizeof(lb_context));

        if (new_context == NULL) {
                syslog(LOG_ERR, "%s: Error allocating memory for lb_context", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }

        new_context->bus = NULL;
        new_context->devices = NULL;
        new_context->devices_size = 0;

        r = _open_system_bus(new_context);
        if(r < 0) {
                syslog(LOG_ERR, "%s: Failed to open system bus: %s", __FUNCTION__, strerror(-r));
                return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_attach_event(new_context->bus, event, SD_EVENT_PRIORITY_NORMAL);
        if (r < 0) {
                syslog(LOG_ERR, "%s: Failed to attach event loop", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }

        *lb_ctx = new_context;
        return LB_SUCCESS;
}

lb_result_t
lb_context_free(lb_context **lb_ctx)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        r = _close_system_bus(*lb_ctx);
        if(r < 0) {
                syslog(LOG_ERR, "%s: Failed to open system bus: %s", __FUNCTION__, strerror(-r));
                return -LB_ERROR_UNSPECIFIED;
        }

        if (lb_ctx != NULL) {

        }
        return LB_SUCCESS;
}

lb_result_t
lb_get_bl_devices(lb_context *lb_ctx, int seconds)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        const char* *objects;
        const char *point;
        int i = 0, r = 0;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(lb_ctx->devices != NULL) {
                free(lb_ctx->devices);
        }

        objects = (const char **)calloc(MAX_OBJECTS, MAX_OBJECTS  *sizeof(const char *));
        if(objects == NULL) {
                syslog(LOG_ERR, "%s: Error allocating memory for objects array", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }

        _scan_devices(lb_ctx, seconds);

        _get_root_objects(lb_ctx, objects);

        if (r < 0) {
              syslog(LOG_ERR, "%s: Error getting root objects", __FUNCTION__);
        //      free(objects);
              return -1;
        }

        while(objects[i] != NULL) {
                if(_is_bl_device(lb_ctx, objects[i])) {
                        _add_new_device(lb_ctx, objects[i]);
                }
                i++;
        }

        free(objects);
        return LB_SUCCESS;
}

lb_result_t
lb_connect_device(lb_context *lb_ctx, bl_device* bl_dev)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(!_is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               BLUEZ_DEST,
                               bl_dev->device_path,
                               BLUEZ_DEVICE,
                               "Connect",
                               &error,
                               &m,
                               NULL);

        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_call_method Connect on device %s failed with error: %s", __FUNCTION__, bl_dev->device_path, error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return LB_SUCCESS;
}

lb_result_t
lb_disconnect_device(lb_context *lb_ctx, bl_device* bl_dev)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(!_is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               BLUEZ_DEST,
                               bl_dev->device_path,
                               BLUEZ_DEVICE,
                               "Disconnect",
                               &error,
                               &m,
                               NULL);

        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_call_method Disconnect on device: %s failed with error: %s", __FUNCTION__, bl_dev->device_path, error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return LB_SUCCESS;
}

lb_result_t
lb_pair_device(lb_context *lb_ctx, bl_device* bl_dev)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(!_is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               BLUEZ_DEST,
                               bl_dev->device_path,
                               BLUEZ_DEVICE,
                               "Pair",
                               &error,
                               &m,
                               NULL);

        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_call_method Pair on device %s failed with error: %s", __FUNCTION__, bl_dev->device_path, error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return LB_SUCCESS;
}

lb_result_t
lb_unpair_device(lb_context *lb_ctx, bl_device* bl_dev)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(!_is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               BLUEZ_DEST,
                               bl_dev->device_path,
                               BLUEZ_DEVICE,
                               "CancelPairing",
                               &error,
                               &m,
                               NULL);

        if(r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_call_method CancelPairing on device %s failed with error: %s", __FUNCTION__, bl_dev->device_path, error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return LB_SUCCESS;
}

lb_result_t
lb_get_ble_device_services(lb_context *lb_ctx, bl_device* bl_dev, ble_service **services)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        const char *point;
        int i = 0, r = 0;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if (!_is_bl_device(lb_ctx, bl_dev->device_path)) {
                syslog(LOG_ERR, "%s: device %s not a bl device", __FUNCTION__, bl_dev->device_path);
                return -LB_ERROR_UNSPECIFIED;
        }

        if (!_is_device_paired(lb_ctx, bl_dev->device_path)) {
                r = lb_pair_device(lb_ctx, bl_dev);
                if (r < 0) {
                        syslog(LOG_ERR, "%s: error pairing device", __FUNCTION__);
                        return -LB_ERROR_UNSPECIFIED;
                }
        }

        if(bl_dev->services != NULL) {
                free(bl_dev->services);
        }
        else {
                bl_dev->services = (ble_service**)malloc(sizeof(ble_service*));
                if (r < 0)
                {
                        exit(r);
                }
                bl_dev->services = NULL;
                bl_dev->services_size = 0;
        }

        const char **objects = (const char **)calloc(MAX_OBJECTS, MAX_OBJECTS  *sizeof(const char *));
        if(objects == NULL) {
                syslog(LOG_ERR, "%s: Error allocating memory for objects array", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = _get_root_objects(lb_ctx, objects);
        if (r < 0) {
              syslog(LOG_ERR, "%s: Error getting root objects", __FUNCTION__);
              //free(objects);
              return -LB_ERROR_UNSPECIFIED;
        }

        while(objects[i] != NULL) {
                if(strstr(objects[i], bl_dev->device_path) && _is_ble_service(lb_ctx, objects[i])) {
                        const char* service_path = objects[i];
                        r = _add_new_service(lb_ctx, bl_dev, service_path);
                        if (r < 0) {
                                syslog(LOG_ERR, "%s: Error adding ble service", __FUNCTION__);
                                continue;
                        }
                        int j = 0;
                        while(objects[j] != NULL) {
                                if(strstr(objects[j], service_path) && _is_ble_characteristic(lb_ctx, objects[j])) {
                                        ble_service *new_service = NULL;
                                        r = lb_get_ble_service_by_service_path(lb_ctx, bl_dev, service_path, &new_service);
                                        if (r < 0) {
                                                syslog(LOG_ERR, "%s: Error getting ble service", __FUNCTION__);
                                                continue;
                                        }
                                        _add_new_characteristic(lb_ctx, new_service, objects[j]);
                                        if (r < 0) {
                                                syslog(LOG_ERR, "%s: Error adding ble characteristic", __FUNCTION__);
                                                continue;
                                        }
                                }
                                j++;
                        }
                }

                i++;
        }

        free(objects);

        return LB_SUCCESS;
}

lb_result_t
lb_get_ble_characteristic_by_characteristic_path(lb_context *lb_ctx, bl_device *bl_dev, const char *characteristic_path, ble_char **ble_characteristic_ret)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i, j;
        for(i = 0; i < bl_dev->services_size; i++) {
                for (j = 0; bl_dev->services[i]->characteristics_size; j++) {
                        if (strncmp(characteristic_path, bl_dev->services[i]->characteristics[j]->char_path, strlen(characteristic_path)) == 0) {
                                *ble_characteristic_ret = bl_dev->services[i]->characteristics[j];
                                return LB_SUCCESS;
                        }
                }
        }
        return -LB_ERROR_UNSPECIFIED;
}

lb_result_t
lb_get_ble_characteristic_by_uuid(lb_context *lb_ctx, bl_device *bl_dev, const char *uuid, ble_char **ble_characteristic_ret)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i, j;

        if (!_is_ble_device(lb_ctx, bl_dev->device_path)) {
                syslog(LOG_ERR, "%s: not a ble device", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }

        for(i = 0; i < bl_dev->services_size; i++) {
                for (j = 0; bl_dev->services_size; j++) {
                        if (strncmp(uuid, bl_dev->services[i]->characteristics[j]->uuid, strlen(uuid)) == 0) {
                                *ble_characteristic_ret = bl_dev->services[i]->characteristics[j];
                                return LB_SUCCESS;
                        }
                }
        }
        return -LB_ERROR_UNSPECIFIED;
}

lb_result_t
lb_get_ble_service_by_service_path(lb_context *lb_ctx, bl_device *bl_dev, const char *service_path, ble_service **ble_service_ret)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i;
        for(i = 0; i < bl_dev->services_size; i++) {
                if (strncmp(service_path, bl_dev->services[i]->service_path, strlen(service_path)) == 0) {
                        *ble_service_ret = bl_dev->services[i];
                        return LB_SUCCESS;
                }
        }
        return -LB_ERROR_UNSPECIFIED;
}

lb_result_t
lb_get_ble_service_by_uuid(lb_context *lb_ctx, bl_device *bl_dev, const char *uuid, ble_service **ble_service_ret)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i;

        if (!_is_ble_device(lb_ctx, bl_dev->device_path)) {
                syslog(LOG_ERR, "%s: not a ble device", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }

        for(i = 0; i < bl_dev->services_size; i++) {
                if (strncmp(uuid, bl_dev->services[i]->uuid, strlen(uuid)) == 0) {
                        *ble_service_ret = bl_dev->services[i];
                        return LB_SUCCESS;
                }
        }
        return -LB_ERROR_UNSPECIFIED;
}

lb_result_t
lb_get_device_by_device_path(lb_context *lb_ctx, const char *device_path, bl_device **bl_device_ret)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i;
        for(i = 0; i < lb_ctx->devices_size; i++) {
                if (strncmp(device_path, lb_ctx->devices[i]->device_path, strlen(device_path)) == 0) {
                        *bl_device_ret = lb_ctx->devices[i];
                        return LB_SUCCESS;
                }
        }
        return -LB_ERROR_UNSPECIFIED;
}

lb_result_t
lb_get_device_by_device_name(lb_context *lb_ctx, const char *name, bl_device **bl_device_ret)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i;
        for(i = 0; i < lb_ctx->devices_size; i++) {
                if (strncmp(name, lb_ctx->devices[i]->name, strlen(name)) == 0) {
                        *bl_device_ret = lb_ctx->devices[i];
                        return LB_SUCCESS;
                }
        }
        return -LB_ERROR_UNSPECIFIED;
}

lb_result_t
lb_get_device_by_device_address(lb_context *lb_ctx, const char *address, bl_device **bl_device_ret)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i;
        for(i = 0; i < lb_ctx->devices_size; i++) {
                if (strncmp(address, lb_ctx->devices[i]->address, strlen(address)) == 0) {
                        *bl_device_ret = lb_ctx->devices[i];
                        return LB_SUCCESS;
                }
        }
        return -LB_ERROR_UNSPECIFIED;
}

lb_result_t
lb_write_to_characteristic(lb_context *lb_ctx, bl_device *bl_dev, const char* uuid, int size, uint8_t *value)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r, i;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;
        ble_char *characteristics = NULL;

        if(!_is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        if (!_is_ble_device(lb_ctx, bl_dev->device_path)) {
                syslog(LOG_ERR, "%s: not a ble device", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = lb_get_ble_characteristic_by_uuid(lb_ctx, bl_dev, uuid, &characteristics);
        if (r < 0) {
                syslog(LOG_ERR, "%s: Failed to get characteristic", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_message_new_method_call(lb_ctx->bus,
                               &m,
                               BLUEZ_DEST,
                               characteristics->char_path,
                               BLUEZ_GATT_CHARACTERISTICS,
                               "WriteValue");
        if (r < 0) {
                syslog(LOG_ERR, "%s: Failed to create message call", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_message_append_array(m, 'y', value, size);
        if (r < 0) {
                syslog(LOG_ERR, "%s: Failed to append array to message call", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_message_append(m, "a{sv}", 0, NULL);
        if (r < 0) {
                syslog(LOG_ERR, "%s: Failed to append a{sv} to message call", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_call(lb_ctx->bus, m, 0, &error, NULL);
        if (r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_call WriteValue on device %s failed with error: %s", __FUNCTION__, characteristics->char_path, error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return LB_SUCCESS;
}

lb_result_t
lb_read_from_characteristic(lb_context *lb_ctx, bl_device *bl_dev, const char* uuid, size_t *size, uint8_t **result)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r, i;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;
        ble_char *characteristics = NULL;

        if(!_is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        if (!_is_ble_device(lb_ctx, bl_dev->device_path)) {
                syslog(LOG_ERR, "%s: not a ble device", __FUNCTION__);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = lb_get_ble_characteristic_by_uuid(lb_ctx, bl_dev, uuid, &characteristics);
        if (r < 0) {
                syslog(LOG_ERR, "%s: Failed to get characteristic", __FUNCTION__);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               BLUEZ_DEST,
                               characteristics->char_path,
                               BLUEZ_GATT_CHARACTERISTICS,
                               "ReadValue",
                               &error,
                               &m,
                               "a{sv}",
                               NULL);
        if (r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_call_method ReadValue on device %s failed with error: %s", __FUNCTION__, characteristics->char_path, error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_message_read_array(m, 'y', (const void**) result, size);
        if (r < 0) {
                syslog(LOG_ERR, "%s: Failed to read byte array message", __FUNCTION__);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return LB_SUCCESS;
}

static int test_callback(sd_bus_message *message, void *userdata, sd_bus_error *error)
{
        printf("callback called\n");
        return LB_SUCCESS;
}

lb_result_t
lb_register_for_device_data(lb_context *lb_ctx, sd_bus_message_handler_t callback, void *userdata)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        r = sd_bus_call_method(lb_ctx->bus,
                               BLUEZ_DEST,
                               "/org/bluez/hci0/dev_98_4F_EE_0F_42_B4/service0009/char000d",
                               BLUEZ_GATT_CHARACTERISTICS,
                               "StartNotify",
                               &error,
                               &m,
                               NULL);
        if (r < 0) {
                syslog(LOG_ERR, "%s: sd_bus_call_method StartNotifying on device %s failed with error: %s", __FUNCTION__, "test", error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_add_match(lb_ctx->bus, NULL, "path='/org/bluez/hci0/dev_98_4F_EE_0F_42_B4/service0009/char000d'", test_callback, userdata);
        if (r < 0) {
                syslog(LOG_ERR, "%s: Failed on sd_bus_add_object", __FUNCTION__);
                return -LB_ERROR_UNSPECIFIED;
        }
        return LB_SUCCESS;
}
