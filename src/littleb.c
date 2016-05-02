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

#include "../include/littleb.h"


const char*
convert_device_path_to_address(const char *address)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        int i;
        const char *prefix = "dev_";

        // find the address in the object string - after "dev_"
        const char  *start_of_address = strstr(address, prefix) + strlen(prefix)  *sizeof(char);

        char *new_address = strdup(start_of_address);

        if(new_address == NULL) {
                syslog(LOG_ERR, "Error copying address to new_addresss\n");
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
is_bus_connected(lb_context *lb_ctx)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        if(lb_ctx->bus == NULL) {
                return false;
        }
        else {
                return (sd_bus_is_open(lb_ctx->bus)) ? true : false;
        }
}

const char*
get_device_name(lb_context *lb_ctx, const char *device_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        sd_bus_error error = SD_BUS_ERROR_NULL;
        int r;
        char *name;

        r = sd_bus_get_property_string(lb_ctx->bus, "org.bluez", device_path, "org.bluez.Device1", "Name", &error, &name);
        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_get_property_string Name on device %s failed with error: %s\n", device_path, error.message);
                sd_bus_error_free(&error);
                return NULL;
        }

        return (const char *) name;
}

const char*
get_device_address(lb_context *lb_ctx, const char *device_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        sd_bus_error error = SD_BUS_ERROR_NULL;
        int r;
        char *address;

        r = sd_bus_get_property_string(lb_ctx->bus, "org.bluez", device_path, "org.bluez.Device1", "Address", &error, &address);
        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_get_property_string Address on device %s failed with error: %s\n", device_path, error.message);
                sd_bus_error_free(&error);
                return NULL;
        }

        return (const char *) address;
}

const char*
get_service_uuid(lb_context *lb_ctx, const char *service_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        sd_bus_error error = SD_BUS_ERROR_NULL;
        int r;
        char *name;

        r = sd_bus_get_property_string(lb_ctx->bus, "org.bluez", service_path, "org.bluez.GattService1", "UUID", &error, &name);
        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_get_property_string UUID on service %s failed with error: %s\n",service_path, error.message);
                sd_bus_error_free(&error);
                return NULL;
        }

        return (const char *) name;
}

const char*
get_characteristic_uuid(lb_context *lb_ctx, const char *characteristic_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        sd_bus_error error = SD_BUS_ERROR_NULL;
        int r;
        char *name;

        r = sd_bus_get_property_string(lb_ctx->bus, "org.bluez", characteristic_path, "org.bluez.GattCharacteristic1", "UUID", &error, &name);
        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_get_property_string UUID on characteristic: %s failed with error: %s\n", characteristic_path, error.message);
                sd_bus_error_free(&error);
                return NULL;
        }

        return (const char *) name;
}


bool
is_string_in_device_introspection(lb_context *lb_ctx, const char *device_path, const char *str)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);        sd_bus_error error = SD_BUS_ERROR_NULL;
        sd_bus_message *m = NULL;

        int r;
        const char *introspect_xml;

        if(!is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "Bus is not opened\n");
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               "org.bluez",
                               device_path,
                               "org.freedesktop.DBus.Introspectable",
                               "Introspect",
                               &error,
                               &m,
                               NULL);
        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_call_method Introspect on device %s failed with error: %s\n", device_path, error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);;
                return false;
        }

        r = sd_bus_message_read_basic(m, 's', &introspect_xml);
        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_message_read_basic failed with error: %s\n", strerror(-r));
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return false;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);

        return (strstr(introspect_xml, str) != NULL) ? true : false;
}

bool
is_bl_device(lb_context *lb_ctx, const char *device_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        return is_string_in_device_introspection(lb_ctx, device_path, "org.bluez.Device1");
}

bool
is_ble_device(lb_context *lb_ctx, const char *device_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        return false;
}

bool
is_ble_service(lb_context *lb_ctx, const char *service_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        return is_string_in_device_introspection(lb_ctx, service_path, "org.bluez.GattService1");
}

bool
is_ble_characteristic(lb_context *lb_ctx, const char *service_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        return is_string_in_device_introspection(lb_ctx, service_path, "org.bluez.GattCharacteristic1");
}

bool
is_service_primary(lb_context *lb_ctx, const char *service_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        sd_bus_error error = SD_BUS_ERROR_NULL;
        int r;
        bool primary;

        r = sd_bus_get_property_trivial(lb_ctx->bus, "org.bluez", service_path, "org.bluez.GattService1", "Primary", &error, 'b', &primary);
        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_get_property_trivial Primary on service %s failed with error: %s\n", service_path, error.message);
                sd_bus_error_free(&error);
                return NULL;
        }

        return primary;
}

int
get_root_objects(lb_context *lb_ctx, const char **objects)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        int r = 0, i = 0;
        const char *device_path;
        sd_bus_error error = SD_BUS_ERROR_NULL;
        sd_bus_message *m = NULL;

        if(!is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "Bus is not opened\n");
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               "org.bluez",
                               "/",
                               "org.freedesktop.DBus.ObjectManager",
                               "GetManagedObjects",
                               &error,
                               &m,
                               NULL);
        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_call_method GetManagedObjects failed with error: %s\n", error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        /* Parse the response message */
        //"a{oa{sa{sv}}}"
        r = sd_bus_message_enter_container(m, 'a', "{oa{sa{sv}}}");
        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_message_enter_container {oa{sa{sv}}} failed with error: %s\n", strerror(-r));
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        while((r = sd_bus_message_enter_container(m, 'e', "oa{sa{sv}}")) > 0) {
                r = sd_bus_message_read_basic(m, 'o', &device_path);
                if(r < 0) {
                        syslog(LOG_ERR, "sd_bus_message_read_basic failed with error: %s\n", strerror(-r));
                                sd_bus_error_free(&error);
        sd_bus_message_unref(m);;
                }
                else {
                        char *restrict new_device_path = malloc(strlen(device_path) + 1);
                        if(new_device_path == NULL) {
                                syslog(LOG_ERR, "Error allocating memory for object name\n");
                                sd_bus_error_free(&error);
                                sd_bus_message_unref(m);
                                return EXIT_FAILURE;
                        }
                        objects[i] = strcpy(new_device_path, device_path);
                        i++;
                }

                r = sd_bus_message_skip(m, "a{sa{sv}}");
                if(r < 0) {
                        syslog(LOG_ERR, "sd_bus_message_skip failed with error: %s\n", strerror(-r));
                        sd_bus_error_free(&error);
                        sd_bus_message_unref(m);
                        return EXIT_FAILURE;
                }

                r = sd_bus_message_exit_container(m);
                if(r < 0) {
                        syslog(LOG_ERR, "sd_bus_message_exit_container oa{sa{sv}} failed with error: %s\n", strerror(-r));
                        sd_bus_error_free(&error);
                        sd_bus_message_unref(m);
                        return EXIT_FAILURE;
                }
        }

        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_message_enter_container oa{sa{sv}} failed with error: %s\n", strerror(-r));
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_message_exit_container(m);
        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_message_exit_container {oa{sa{sv}}} failed with error: %s\n", strerror(-r));
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
add_new_characteristic(lb_context *lb_ctx, ble_service *service, const char *characteristic_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);

        int current_index = service->characteristics_size;
        if(service->characteristics_size == 0 || service->characteristics == NULL) {
                service->characteristics = malloc(sizeof(ble_char*));
                service->characteristics_size++;
        }
        else {
                service->characteristics_size++;
                service->characteristics = realloc(service->characteristics, (service->characteristics_size) * sizeof(ble_char*));
                if(service->characteristics == NULL) {
                        syslog(LOG_ERR, "Error reallocating memory for characteristics\n");
                        return EXIT_FAILURE;
                }
        }

        ble_char *new_characteristic= malloc(sizeof(ble_char));

        new_characteristic->char_path = characteristic_path;

        const char *uuid = get_characteristic_uuid(lb_ctx, characteristic_path);
        if(uuid == NULL) {
                syslog(LOG_ERR, "Error couldn't find characteristic uuid\n");
                new_characteristic->uuid = "null";
        }
        else {
                new_characteristic->uuid = uuid;
        }

        service->characteristics[current_index] = new_characteristic;

        return EXIT_SUCCESS;
}

int
add_new_service(lb_context *lb_ctx, bl_device* bl_dev, const char *service_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);

        int current_index = bl_dev->services_size;
        if(bl_dev->services_size == 0 || bl_dev->services == NULL) {
                bl_dev->services = malloc(sizeof(ble_service*));
                bl_dev->services_size++;
        }
        else {
                bl_dev->services_size++;
                bl_dev->services = realloc(bl_dev->services, (bl_dev->services_size) * sizeof(ble_service*));
                if(bl_dev->services == NULL) {
                        syslog(LOG_ERR, "Error reallocating memory for services\n");
                        return EXIT_FAILURE;
                }
        }

        ble_service *new_service = malloc(sizeof(ble_service));

        new_service->service_path = service_path;

        const char *uuid = get_service_uuid(lb_ctx, service_path);
        if(uuid == NULL) {
                syslog(LOG_ERR, "Error couldn't find service uuid\n");
                new_service->uuid = "null";
        }
        else {
                new_service->uuid = uuid;
        }

        new_service->primary = is_service_primary(lb_ctx, service_path);

        new_service->characteristics = NULL;
        new_service->characteristics_size = 0;

        bl_dev->services[current_index] = new_service;

        return EXIT_SUCCESS;
}

int
add_new_device(lb_context *lb_ctx, const char *device_path)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);

        int current_index = lb_ctx->devices_size;
        if(lb_ctx->devices_size == 0 || lb_ctx->devices == NULL) {
                lb_ctx->devices = malloc(sizeof(bl_device*));
                lb_ctx->devices_size++;
        }
        else {
                lb_ctx->devices_size++;
                lb_ctx->devices = realloc(lb_ctx->devices, (lb_ctx->devices_size)  *sizeof(bl_device*));
                if(lb_ctx->devices == NULL) {
                        syslog(LOG_ERR, "Error reallocating memory for devices\n");
                        return EXIT_FAILURE;
                }
        }

        bl_device *new_device = malloc(sizeof(bl_device));

        new_device->device_path = device_path;
        const char *name = get_device_name(lb_ctx, device_path);
        if(name == NULL) {
                syslog(LOG_ERR, "Error couldn't find device name\n");
                new_device->name = "null";
        }
        else {
                new_device->name = name;
        }
        const char *address = get_device_address(lb_ctx, device_path);
        if(address == NULL) {
                syslog(LOG_ERR, "Error couldn't find device address\n");
                new_device->address = "null";
        }
        else {
                new_device->address = address;
        }

        new_device->services = NULL;
        new_device->services_size = 0;

        //new_device->address = convert_device_path_to_address(device_path);
        lb_ctx->devices[current_index] = new_device;

        return EXIT_SUCCESS;
}

int
scan_devices(lb_context *lb_ctx, int seconds)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        int r = 0;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(!is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "Bus is not opened\n");
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_call_method(lb_ctx->bus, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", "StartDiscovery", &error, &m,
        NULL);
        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_call_method StartDiscovery failed with error: %s\n", error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sleep(seconds);

        r = sd_bus_call_method(lb_ctx->bus, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", "StopDiscovery", &error, &m,
        NULL);

        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_call_method StopDiscovery failed with error: %s\n", error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
lb_context_new(lb_context **lb_ctx)
{
        int r = 0;

        if(*lb_ctx != NULL) {
                syslog(LOG_ERR, "lb_context is not empty!:\n");
                return EXIT_FAILURE;
        }

        lb_context *new_context = malloc(sizeof(lb_context));

        if (new_context != NULL) {
                new_context->bus = NULL;
                new_context->devices = NULL;
                new_context->devices_size = 0;
        }
        else {
                syslog(LOG_ERR, "Error allocating memory for lb_context:\n");
                return EXIT_FAILURE;
        }

        *lb_ctx = new_context;
        return EXIT_SUCCESS;
}

int
lb_open_system_bus(lb_context **lb_ctx)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        lb_context *new_context = NULL;

        r = lb_context_new(&new_context);
        if(r < 0) {
                syslog(LOG_ERR, "Failed to create new lb context\n");
        }

        /* Connect to the system bus */
        r = sd_bus_open_system(&new_context->bus);
        if(r < 0) {
                syslog(LOG_ERR, "Failed to connect to system bus: %s\n", strerror(-r));
        }

        *lb_ctx = new_context;
        return r;
}

int
lb_close_system_bus(lb_context *lb_ctx)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        sd_bus_unref(lb_ctx->bus);
        if(lb_ctx->devices) {
                free(lb_ctx->devices);
                lb_ctx->devices_size = 0;
        }
        return EXIT_SUCCESS;
}

int
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

        objects = malloc(MAX_OBJECTS  *sizeof(const char *));
        if(objects == NULL) {
                syslog(LOG_ERR, "Error allocating memory for objects array\n");
                return EXIT_FAILURE;
        }

        scan_devices(lb_ctx, seconds);

        get_root_objects(lb_ctx, objects);

        //if (r < 0) {
        //      syslog(LOG_ERR, "Error getting root objects\n");
        //      free(objects);
        //      return -1;
        //}

        while(objects[i] != NULL) {
                if(is_bl_device(lb_ctx, objects[i])) {
                        add_new_device(lb_ctx, objects[i]);
                }
                i++;
        }

        free(objects);
        return EXIT_SUCCESS;
}

int
lb_connect_device(lb_context *lb_ctx, bl_device* bl_dev)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(!is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "Bus is not opened\n");
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               "org.bluez",
                               bl_dev->device_path,
                               "org.bluez.Device1",
                               "Connect",
                               &error,
                               &m,
                               NULL);

        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_call_method Connect on device %s failed with error: %s\n", bl_dev->device_path, error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
lb_disconnect_device(lb_context *lb_ctx, bl_device* bl_dev)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(!is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "Bus is not opened\n");
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               "org.bluez",
                               bl_dev->device_path,
                               "org.bluez.Device1",
                               "Disconnect",
                               &error,
                               &m,
                               NULL);

        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_call_method Disconnect on device: %s failed with error: %s\n", bl_dev->device_path, error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
lb_pair_device(lb_context *lb_ctx, bl_device* bl_dev)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(!is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "Bus is not opened\n");
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               "org.bluez",
                               bl_dev->device_path,
                               "org.bluez.Device1",
                               "Pair",
                               &error,
                               &m,
                               NULL);

        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_call_method Pair on device %s failed with error: %s\n", bl_dev->device_path, error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
lb_unpair_device(lb_context *lb_ctx, bl_device* bl_dev)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(!is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "Bus is not opened\n");
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               "org.bluez",
                               bl_dev->device_path,
                               "org.bluez.Device1",
                               "CancelPairing",
                               &error,
                               &m,
                               NULL);

        if(r < 0) {
                syslog(LOG_ERR, "sd_bus_call_method CancelPairing on device %s failed with error: %s\n", bl_dev->device_path, error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
lb_get_ble_device_services(lb_context *lb_ctx, bl_device* bl_dev, ble_service **services)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        const char *point;
        int i = 0, r = 0;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(bl_dev->services != NULL) {
                free(bl_dev->services);
        }
        else {
                bl_dev->services = malloc(sizeof(lb_context));
                if (r < 0)
                {
                        exit(r);
                }
                bl_dev->services = NULL;
                bl_dev->services_size = 0;
        }

        const char **objects = malloc(MAX_OBJECTS  *sizeof(const char *));
        if(objects == NULL) {
                syslog(LOG_ERR, "Error allocating memory for objects array\n");
                return EXIT_FAILURE;
        }

        r = get_root_objects(lb_ctx, objects);
        if (r < 0) {
              syslog(LOG_ERR, "Error getting root objects\n");
              //free(objects);
              return EXIT_FAILURE;
        }

        while(objects[i] != NULL) {
                if(strstr(objects[i], bl_dev->device_path) && is_ble_service(lb_ctx, objects[i])) {
                        const char* service_path = objects[i];
                        r = add_new_service(lb_ctx, bl_dev, service_path);
                        if (r < 0) {
                                syslog(LOG_ERR, "Error adding ble service\n");
                                continue;
                        }
                        int j = 0;
                        while(objects[j] != NULL) {
                                if(strstr(objects[j], service_path) && is_ble_characteristic(lb_ctx, objects[j])) {
                                        ble_service *new_service = NULL;
                                        r = lb_get_ble_service_by_service_path(lb_ctx, bl_dev, service_path, &new_service);
                                        if (r < 0) {
                                                syslog(LOG_ERR, "Error getting ble service\n");
                                                continue;
                                        }
                                        add_new_characteristic(lb_ctx, new_service, objects[j]);
                                        if (r < 0) {
                                                syslog(LOG_ERR, "Error adding ble characteristic\n");
                                                continue;
                                        }
                                }
                                j++;
                        }
                }

                i++;
        }

        free(objects);

        return EXIT_SUCCESS;
}

int
lb_get_ble_characteristic_by_characteristic_path(lb_context *lb_ctx, bl_device *bl_dev, const char *characteristic_path, ble_char **ble_characteristic_ret)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i, j;
        for(i = 0; i < bl_dev->services_size; i++) {
                for (j = 0; bl_dev->services[i]->characteristics_size; j++) {
                        if (strncmp(characteristic_path, bl_dev->services[i]->characteristics[j]->char_path, strlen(characteristic_path)) == 0) {
                                *ble_characteristic_ret = bl_dev->services[i]->characteristics[j];
                                return EXIT_SUCCESS;
                        }
                }
        }
        return EXIT_FAILURE;
}

int
lb_get_ble_characteristic_by_uuid(lb_context *lb_ctx, bl_device *bl_dev, const char *uuid, ble_char **ble_characteristic_ret)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i, j;
        for(i = 0; i < bl_dev->services_size; i++) {
                for (j = 0; bl_dev->services_size; j++) {
                        if (strncmp(uuid, bl_dev->services[i]->characteristics[j]->uuid, strlen(uuid)) == 0) {
                                *ble_characteristic_ret = bl_dev->services[i]->characteristics[j];
                                return EXIT_SUCCESS;
                        }
                }
        }
        return EXIT_FAILURE;
}

int
lb_get_ble_service_by_service_path(lb_context *lb_ctx, bl_device *bl_dev, const char *service_path, ble_service **ble_service_ret)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i;
        for(i = 0; i < bl_dev->services_size; i++) {
                if (strncmp(service_path, bl_dev->services[i]->service_path, strlen(service_path)) == 0) {
                        *ble_service_ret = bl_dev->services[i];
                        return EXIT_SUCCESS;
                }
        }
        return EXIT_FAILURE;
}

int
lb_get_ble_service_by_uuid(lb_context *lb_ctx, bl_device *bl_dev, const char *uuid, ble_service **ble_service_ret)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i;
        for(i = 0; i < bl_dev->services_size; i++) {
                if (strncmp(uuid, bl_dev->services[i]->uuid, strlen(uuid)) == 0) {
                        *ble_service_ret = bl_dev->services[i];
                        return EXIT_SUCCESS;
                }
        }
        return EXIT_FAILURE;
}

int
lb_get_device_by_device_path(lb_context *lb_ctx, const char *device_path, bl_device **bl_device_ret)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i;
        for(i = 0; i < lb_ctx->devices_size; i++) {
                if (strncmp(device_path, lb_ctx->devices[i]->device_path, strlen(device_path)) == 0) {
                        *bl_device_ret = lb_ctx->devices[i];
                        return EXIT_SUCCESS;
                }
        }
        return EXIT_FAILURE;
}

int
lb_get_device_by_device_name(lb_context *lb_ctx, const char *name, bl_device **bl_device_ret)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i;
        for(i = 0; i < lb_ctx->devices_size; i++) {
                if (strncmp(name, lb_ctx->devices[i]->name, strlen(name)) == 0) {
                        *bl_device_ret = lb_ctx->devices[i];
                        return EXIT_SUCCESS;
                }
        }
        return EXIT_FAILURE;
}

int
lb_get_device_by_device_address(lb_context *lb_ctx, const char *address, bl_device **bl_device_ret)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i;
        for(i = 0; i < lb_ctx->devices_size; i++) {
                if (strncmp(address, lb_ctx->devices[i]->address, strlen(address)) == 0) {
                        *bl_device_ret = lb_ctx->devices[i];
                        return EXIT_SUCCESS;
                }
        }
        return EXIT_FAILURE;
}

int
lb_write_to_characteristic(lb_context *lb_ctx, bl_device *bl_dev, const char* uuid, int size, uint8_t *value)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r, i;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;
        ble_char *characteristics = NULL;


        if(!is_bus_connected(lb_ctx)) {
                syslog(LOG_ERR, "Bus is not opened\n");
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = lb_get_ble_characteristic_by_uuid(lb_ctx, bl_dev, uuid, &characteristics);
        if (r < 0) {
                syslog(LOG_ERR, "Failed to get characteristic\n");
                return EXIT_FAILURE;
        }

        for (i = 0; i < size; i++) {
                r = sd_bus_call_method(lb_ctx->bus,
                                       "org.bluez",
                                       characteristics->char_path,
                                       "org.bluez.GattCharacteristic1",
                                       "WriteValue",
                                       &error,
                                       &m,
                                       "ay",
                                       1,
                                       value[i]);

                if(r < 0) {
                        syslog(LOG_ERR, "sd_bus_call_method WriteValue on characteristic %s failed with error: %s\n", characteristics->char_path, error.message);
                        sd_bus_error_free(&error);
                        sd_bus_message_unref(m);
                        return EXIT_FAILURE;
                }
                sleep(0.1);
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}
