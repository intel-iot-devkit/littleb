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
#include "littleb_internal_types.h"

static sd_event* event = NULL;
static pthread_t event_thread;
static event_matches_callbacks** events_matches_array = NULL;
static uint event_arr_size = 0;
static lb_context lb_ctx = NULL;

void*
_run_event_loop(void* arg)
{
    int r, i;
    sd_bus* bus = NULL;

    r = sd_bus_open_system(&bus);
    if (r < 0) {
        fprintf(stderr, "%s: Failed to connect to system bus: %s", __FUNCTION__, strerror(-r));
        return NULL;
    }

    r = sd_event_default(&event);
    if (r < 0) {
        syslog(LOG_ERR, "%s: Failed to set default event", __FUNCTION__);
        return NULL;
    }

    r = sd_bus_attach_event(bus, event, SD_EVENT_PRIORITY_NORMAL);
    if (r < 0) {
        syslog(LOG_ERR, "%s: Failed to attach event loop", __FUNCTION__);
        return NULL;
    }

    for (i = 0; i < event_arr_size; i++) {
        r = sd_bus_add_match(bus, NULL, events_matches_array[i]->event,
                             *(events_matches_array[i]->callback), events_matches_array[i]->userdata);
        if (r < 0) {
            syslog(LOG_ERR, "%s: Failed on sd_bus_add_match with error %d", __FUNCTION__, r);
            return NULL;
        }
    }

    r = sd_event_loop(event);
    if (r < 0) {
        syslog(LOG_ERR, "%s: Failed to run event loop: %s", __FUNCTION__, strerror(-r));
        return NULL;
    }

    return NULL;
}

const char*
_convert_device_path_to_address(const char* address)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int i;
    const char* prefix = "dev_";

    // find the address in the object string - after "dev_"
    const char* start_of_address = strstr(address, prefix) + strlen(prefix) * sizeof(char);

    char* new_address = strdup(start_of_address);
    if (new_address == NULL) {
        syslog(LOG_ERR, "%s: Error copying address to new_addresss", __FUNCTION__);
        return NULL;
    }

    for (i = 0; i < strlen(new_address); i++) {
        if (new_address[i] == '_') {
            new_address[i] = ':';
        }
    }

    return (const char*) new_address;
}

bool
_is_bus_connected()
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    if (lb_ctx->bus == NULL) {
        return false;
    } else {
        return (sd_bus_is_open(lb_ctx->bus)) ? true : false;
    }
}

lb_result_t
_get_root_objects(const char** objects)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int r = 0, i = 0;
    const char* device_path;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = NULL;

    if (!_is_bus_connected(lb_ctx)) {
        syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
        sd_bus_error_free(&error);
        return -LB_ERROR_INVALID_BUS;
    }

    r = sd_bus_call_method(lb_ctx->bus, BLUEZ_DEST, "/", "org.freedesktop.DBus.ObjectManager",
                           "GetManagedObjects", &error, &reply, NULL);
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_call_method GetManagedObjects failed with error: %s",
               __FUNCTION__, error.message);
        sd_bus_error_free(&error);
        sd_bus_message_unref(reply);
        return -LB_ERROR_SD_BUS_CALL_FAIL;
    }

    /* Parse the response message */
    //"a{oa{sa{sv}}}"
    r = sd_bus_message_enter_container(reply, 'a', "{oa{sa{sv}}}");
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_message_enter_container {oa{sa{sv}}} failed with error: %s",
               __FUNCTION__, strerror(-r));
        sd_bus_error_free(&error);
        sd_bus_message_unref(reply);
        return -LB_ERROR_UNSPECIFIED;
    }

    while ((r = sd_bus_message_enter_container(reply, 'e', "oa{sa{sv}}")) > 0) {
        r = sd_bus_message_read_basic(reply, 'o', &device_path);
        if (r < 0) {
            syslog(LOG_ERR, "%s: sd_bus_message_read_basic failed with error: %s", __FUNCTION__, strerror(-r));
            sd_bus_error_free(&error);
            sd_bus_message_unref(reply);
        } else {
            char* new_device_path = (char*) malloc(strlen(device_path) + 1);
            if (new_device_path == NULL) {
                syslog(LOG_ERR, "%s: Error allocating memory for object name", __FUNCTION__);
                sd_bus_error_free(&error);
                sd_bus_message_unref(reply);
                return -LB_ERROR_MEMEORY_ALLOCATION;
            }
            objects[i] = strcpy(new_device_path, device_path);
            i++;
        }

        r = sd_bus_message_skip(reply, "a{sa{sv}}");
        if (r < 0) {
            syslog(LOG_ERR, "%s: sd_bus_message_skip failed with error: %s", __FUNCTION__, strerror(-r));
            sd_bus_error_free(&error);
            sd_bus_message_unref(reply);
            return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_message_exit_container(reply);
        if (r < 0) {
            syslog(LOG_ERR, "%s: sd_bus_message_exit_container oa{sa{sv}} failed with error: %s",
                   __FUNCTION__, strerror(-r));
            sd_bus_error_free(&error);
            sd_bus_message_unref(reply);
            return -LB_ERROR_UNSPECIFIED;
        }
    }

    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_message_enter_container oa{sa{sv}} failed with error: %s",
               __FUNCTION__, strerror(-r));
        sd_bus_error_free(&error);
        sd_bus_message_unref(reply);
        return -LB_ERROR_UNSPECIFIED;
    }

    r = sd_bus_message_exit_container(reply);
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_message_exit_container {oa{sa{sv}}} failed with error: %s",
               __FUNCTION__, strerror(-r));
        sd_bus_error_free(&error);
        sd_bus_message_unref(reply);
        return -LB_ERROR_UNSPECIFIED;
    }

    sd_bus_error_free(&error);
    sd_bus_message_unref(reply);
    return LB_SUCCESS;
}

const char*
_get_device_name(const char* device_path)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    sd_bus_error error = SD_BUS_ERROR_NULL;
    int r;
    char* name;

    r = sd_bus_get_property_string(lb_ctx->bus, BLUEZ_DEST, device_path, BLUEZ_DEVICE, "Name", &error, &name);
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_get_property_string Name on device %s failed with error: %s",
               __FUNCTION__, device_path, error.message);
        sd_bus_error_free(&error);
        return NULL;
    }

    sd_bus_error_free(&error);
    return (const char*) name;
}

const char*
_get_device_address(const char* device_path)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    sd_bus_error error = SD_BUS_ERROR_NULL;
    int r;
    char* address;

    r = sd_bus_get_property_string(lb_ctx->bus, BLUEZ_DEST, device_path, BLUEZ_DEVICE, "Address",
                                   &error, &address);
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_get_property_string Address on device %s failed with error: %s",
               __FUNCTION__, device_path, error.message);
        sd_bus_error_free(&error);
        return NULL;
    }

    sd_bus_error_free(&error);
    return (const char*) address;
}

const char*
_get_service_uuid(const char* service_path)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    sd_bus_error error = SD_BUS_ERROR_NULL;
    int r;
    char* name;

    r = sd_bus_get_property_string(lb_ctx->bus, BLUEZ_DEST, service_path, "org.bluez.GattService1",
                                   "UUID", &error, &name);
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_get_property_string UUID on service %s failed with error: %s",
               __FUNCTION__, service_path, error.message);
        sd_bus_error_free(&error);
        return NULL;
    }

    sd_bus_error_free(&error);
    return (const char*) name;
}

const char*
_get_characteristic_uuid(const char* characteristic_path)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    sd_bus_error error = SD_BUS_ERROR_NULL;
    int r;
    char* name;

    r = sd_bus_get_property_string(lb_ctx->bus, BLUEZ_DEST, characteristic_path,
                                   BLUEZ_GATT_CHARACTERISTICS, "UUID", &error, &name);
    if (r < 0) {
        syslog(LOG_ERR,
               "%s: sd_bus_get_property_string UUID on characteristic: %s failed with error: %s",
               __FUNCTION__, characteristic_path, error.message);
        sd_bus_error_free(&error);
        return NULL;
    }

    sd_bus_error_free(&error);
    return (const char*) name;
}

bool
_is_string_in_device_introspection(const char* device_path, const char* str)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message* reply = NULL;

    int r;
    bool result = false;
    const char* introspect_xml;

    if (!_is_bus_connected(lb_ctx)) {
        syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
        sd_bus_error_free(&error);
        return -LB_ERROR_INVALID_BUS;
    }

    r = sd_bus_call_method(lb_ctx->bus, BLUEZ_DEST, device_path,
                           "org.freedesktop.DBus.Introspectable", "Introspect", &error, &reply, NULL);
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_call_method Introspect on device %s failed with error: %s",
               __FUNCTION__, device_path, error.message);
        sd_bus_error_free(&error);
        sd_bus_message_unref(reply);
        return false;
    }

    r = sd_bus_message_read_basic(reply, 's', &introspect_xml);
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_message_read_basic failed with error: %s", __FUNCTION__, strerror(-r));
        sd_bus_error_free(&error);
        sd_bus_message_unref(reply);
        return false;
    }

    result = (strstr(introspect_xml, str) != NULL) ? true : false;

    sd_bus_error_free(&error);
    sd_bus_message_unref(reply);

    return result;
}

bool
_is_bl_device(const char* device_path)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    return _is_string_in_device_introspection(device_path, BLUEZ_DEVICE);
}

bool
_is_ble_device(const char* device_path)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    const char** objects;
    int i = 0, r = 0;
    bool result = false;

    if (!_is_bl_device(device_path)) {
        syslog(LOG_ERR, "%s: not a bl device", __FUNCTION__);
        return false;
    }

    lb_bl_device* device = NULL;
    r = lb_get_device_by_device_path(device_path, &device);
    if (r < 0) {
        fprintf(stderr, "ERROR: Device %s not found\n", device_path);
    }

    if (device != NULL && device->services_size > 0)
        return true;

    objects = (const char**) calloc(MAX_OBJECTS, MAX_OBJECTS * sizeof(const char*));
    if (objects == NULL) {
        syslog(LOG_ERR, "%s: Error allocating memory for objects array", __FUNCTION__);
        return -LB_ERROR_MEMEORY_ALLOCATION;
    }

    r = _get_root_objects(objects);
    if (r < 0) {
        syslog(LOG_ERR, "%s: Error getting root objects", __FUNCTION__);
        return false;
    }

    while (objects[i] != NULL) {
        if ((strstr(objects[i], device_path) != NULL) && (strstr(objects[i], "service") != NULL)) {
            result = true;
        }
        free((char*) objects[i]);
        i++;
    }
    free(objects);

    return result;
}

bool
_is_ble_service(const char* service_path)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    return _is_string_in_device_introspection(service_path, BLUEZ_GATT_SERVICE);
}

bool
_is_ble_characteristic(const char* service_path)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    return _is_string_in_device_introspection(service_path, BLUEZ_GATT_CHARACTERISTICS);
}

bool
_is_service_primary(const char* service_path)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    sd_bus_error error = SD_BUS_ERROR_NULL;
    int r;
    bool primary;

    r = sd_bus_get_property_trivial(lb_ctx->bus, BLUEZ_DEST, service_path, "org.bluez.GattService1",
                                    "Primary", &error, 'b', &primary);
    if (r < 0) {
        syslog(LOG_ERR,
               "%s: sd_bus_get_property_trivial Primary on service %s failed with error: %s",
               __FUNCTION__, service_path, error.message);
        sd_bus_error_free(&error);
        return NULL;
    }

    sd_bus_error_free(&error);
    return primary;
}

bool
_is_device_paired(const char* device_path)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    sd_bus_error error = SD_BUS_ERROR_NULL;
    int r;
    bool paired;

    r = sd_bus_get_property_trivial(lb_ctx->bus, BLUEZ_DEST, device_path, BLUEZ_DEVICE, "Paired",
                                    &error, 'b', &paired);
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_get_property_trivial Paired on device %s failed with error: %s",
               __FUNCTION__, device_path, error.message);
        sd_bus_error_free(&error);
        return NULL;
    }

    sd_bus_error_free(&error);
    return paired;
}

lb_result_t
_add_new_characteristic(lb_ble_service* service, const char* characteristic_path)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int current_index = service->characteristics_size;
    if (service->characteristics_size == 0 || service->characteristics == NULL) {
        service->characteristics = (lb_ble_char**) malloc(sizeof(lb_ble_char*));
        if (service->characteristics == NULL) {
            syslog(LOG_ERR, "%s: Error allocating memory for characteristics", __FUNCTION__);
            return -LB_ERROR_MEMEORY_ALLOCATION;
        }
        service->characteristics_size++;
    } else {
        service->characteristics_size++;
        service->characteristics =
        realloc(service->characteristics, (service->characteristics_size) * sizeof(lb_ble_char*));
        if (service->characteristics == NULL) {
            syslog(LOG_ERR, "%s: Error reallocating memory for characteristics", __FUNCTION__);
            return -LB_ERROR_MEMEORY_ALLOCATION;
        }
    }

    lb_ble_char* new_characteristic = (lb_ble_char*) malloc(sizeof(lb_ble_char));
    if (new_characteristic == NULL) {
        syslog(LOG_ERR, "%s: Error allocating memory for new characteristic", __FUNCTION__);
        service->characteristics_size--;
        return -LB_ERROR_MEMEORY_ALLOCATION;
    }

    new_characteristic->char_path = strdup(characteristic_path);
    if (new_characteristic->char_path == NULL) {
        syslog(LOG_ERR, "%s: Error allocating memory for new characteristic", __FUNCTION__);
        service->characteristics_size--;
        return -LB_ERROR_MEMEORY_ALLOCATION;
    }

    const char* uuid = _get_characteristic_uuid(characteristic_path);
    if (uuid == NULL) {
        syslog(LOG_ERR, "%s: Error couldn't find characteristic uuid", __FUNCTION__);
        new_characteristic->uuid = strdup("null");
        if (new_characteristic->uuid == NULL) {
            syslog(LOG_ERR, "%s: Error allocating memory for new characteristic uuid", __FUNCTION__);
            service->characteristics_size--;
            return -LB_ERROR_MEMEORY_ALLOCATION;
        }
    } else {
        new_characteristic->uuid = uuid;
    }

    service->characteristics[current_index] = new_characteristic;

    return LB_SUCCESS;
}

lb_result_t
_add_new_service(lb_bl_device* dev, const char* service_path)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    if (!_is_ble_device(dev->device_path)) {
        syslog(LOG_ERR, "%s: not a ble device", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    int current_index = dev->services_size;
    if (dev->services_size == 0 || dev->services == NULL) {
        dev->services = (lb_ble_service**) malloc(sizeof(lb_ble_service*));
        if (dev->services == NULL) {
            syslog(LOG_ERR, "%s: Error allocating memory for services", __FUNCTION__);
            return -LB_ERROR_MEMEORY_ALLOCATION;
        }
        dev->services_size++;
    } else {
        dev->services_size++;
        dev->services = realloc(dev->services, (dev->services_size) * sizeof(lb_ble_service*));
        if (dev->services == NULL) {
            syslog(LOG_ERR, "%s: Error reallocating memory for services", __FUNCTION__);
            return -LB_ERROR_MEMEORY_ALLOCATION;
        }
    }

    lb_ble_service* new_service = (lb_ble_service*) malloc(sizeof(lb_ble_service));
    if (new_service == NULL) {
        syslog(LOG_ERR, "%s: Error allocating memory for new_service", __FUNCTION__);
        dev->services_size--;
        return -LB_ERROR_MEMEORY_ALLOCATION;
    }

    new_service->service_path = strdup(service_path);
    if (new_service->service_path == NULL) {
        syslog(LOG_ERR, "%s: Error allocating memory for new service", __FUNCTION__);
        return -LB_ERROR_MEMEORY_ALLOCATION;
    }

    const char* uuid = _get_service_uuid(service_path);
    if (uuid == NULL) {
        syslog(LOG_ERR, "%s: Error couldn't find service uuid", __FUNCTION__);
        new_service->uuid = strdup("null");
        if (new_service->uuid == NULL) {
            syslog(LOG_ERR, "%s: Error allocating memory for new service uuid", __FUNCTION__);
            dev->services_size--;
            return -LB_ERROR_MEMEORY_ALLOCATION;
        }
    } else {
        new_service->uuid = uuid;
    }

    new_service->primary = _is_service_primary(service_path);

    new_service->characteristics = NULL;
    new_service->characteristics_size = 0;

    dev->services[current_index] = new_service;

    return LB_SUCCESS;
}

lb_result_t
_add_new_device(const char* device_path)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int current_index = lb_ctx->devices_size;
    if (lb_ctx->devices_size == 0 || lb_ctx->devices == NULL) {
        lb_ctx->devices = (lb_bl_device**) malloc(sizeof(lb_bl_device*));
        if (lb_ctx->devices == NULL) {
            syslog(LOG_ERR, "%s: Error allocating memory for devices", __FUNCTION__);
            return -LB_ERROR_MEMEORY_ALLOCATION;
        }
        lb_ctx->devices_size++;
    } else {
        lb_ctx->devices_size++;
        lb_ctx->devices = realloc(lb_ctx->devices, (lb_ctx->devices_size) * sizeof(lb_bl_device*));
        if (lb_ctx->devices == NULL) {
            syslog(LOG_ERR, "%s: Error reallocating memory for devices", __FUNCTION__);
            return -LB_ERROR_MEMEORY_ALLOCATION;
        }
    }

    lb_bl_device* new_device = (lb_bl_device*) malloc(sizeof(lb_bl_device));
    if (new_device == NULL) {
        syslog(LOG_ERR, "%s: Error allocating memory for new_device", __FUNCTION__);
        lb_ctx->devices_size--;
        return -LB_ERROR_MEMEORY_ALLOCATION;
    }

    new_device->device_path = strdup(device_path);
    if (new_device->device_path == NULL) {
        syslog(LOG_ERR, "%s: Error allocating memory for new_device", __FUNCTION__);
        lb_ctx->devices_size--;
        return -LB_ERROR_MEMEORY_ALLOCATION;
    }

    const char* name = _get_device_name(device_path);
    if (name == NULL) {
        syslog(LOG_ERR, "%s: Error couldn't find device name", __FUNCTION__);
        new_device->name = strdup("null");
        if (new_device->name == NULL) {
            syslog(LOG_ERR, "%s: Error allocating memory for new device", __FUNCTION__);
            lb_ctx->devices_size--;
            return -LB_ERROR_MEMEORY_ALLOCATION;
        }

    } else {
        new_device->name = name;
    }
    const char* address = _get_device_address(device_path);
    if (address == NULL) {
        syslog(LOG_ERR, "%s: Error couldn't find device address", __FUNCTION__);
        new_device->address = "null";
    } else {
        new_device->address = address;
    }

    new_device->services = NULL;
    new_device->services_size = 0;

    // new_device->address = convert_device_path_to_address(device_path);
    lb_ctx->devices[current_index] = new_device;

    return LB_SUCCESS;
}

lb_result_t
_scan_devices(int seconds)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int r = 0;
    sd_bus_error error = SD_BUS_ERROR_NULL;

    if (!_is_bus_connected(lb_ctx)) {
        syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
        sd_bus_error_free(&error);
        return -LB_ERROR_INVALID_BUS;
    }

    r = sd_bus_call_method(lb_ctx->bus, BLUEZ_DEST, "/org/bluez/hci0", "org.bluez.Adapter1",
                           "StartDiscovery", &error, NULL, NULL);
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_call_method StartDiscovery failed with error: %s", __FUNCTION__,
               error.message);
        sd_bus_error_free(&error);
        return -LB_ERROR_SD_BUS_CALL_FAIL;
    }

    sleep(seconds);

    r = sd_bus_call_method(lb_ctx->bus, BLUEZ_DEST, "/org/bluez/hci0", "org.bluez.Adapter1",
                           "StopDiscovery", &error, NULL, NULL);
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_call_method StopDiscovery failed with error: %s", __FUNCTION__,
               error.message);
        sd_bus_error_free(&error);
        return -LB_ERROR_SD_BUS_CALL_FAIL;
    }

    sd_bus_error_free(&error);
    return LB_SUCCESS;
}

lb_result_t
_open_system_bus()
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int r;

    /* Connect to the system bus */
    r = sd_bus_open_system(&(lb_ctx->bus));
    if (r < 0) {
        syslog(LOG_ERR, "%s: Failed to connect to system bus: %s", __FUNCTION__, strerror(-r));
    }

    return r;
}

lb_result_t
_close_system_bus()
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif

    sd_bus_unref(lb_ctx->bus);
    return LB_SUCCESS;
}

lb_result_t
lb_context_new()
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int r = 0;

    if (lb_ctx != NULL) {
        syslog(LOG_ERR, "%s: Context already initialized", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    lb_ctx = (lb_context) malloc(sizeof(struct bl_context));
    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: Error allocating memory for lb_context", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    lb_ctx->bus = NULL;
    lb_ctx->devices = NULL;
    lb_ctx->devices_size = 0;

    r = _open_system_bus();
    if (r < 0) {
        syslog(LOG_ERR, "%s: Failed to open system bus: %s", __FUNCTION__, strerror(-r));
        return -LB_ERROR_INVALID_CONTEXT;
    }

    return LB_SUCCESS;
}

lb_result_t
lb_context_free()
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int r = 0, i = 0, j = 0, k = 0;

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    r = _close_system_bus(lb_ctx);
    if (r < 0) {
        syslog(LOG_ERR, "%s: Failed to close system bus: %s", __FUNCTION__, strerror(-r));
        return -LB_ERROR_UNSPECIFIED;
    }

    for (i = 0; i < lb_ctx->devices_size; i++) {
        for (j = 0; j < lb_ctx->devices[i]->services_size; j++) {
            for (k = 0; k < lb_ctx->devices[i]->services[j]->characteristics_size; k++) {
                if (lb_ctx->devices[i]->services[j]->characteristics[k]->char_path != NULL)
                    free((char*) lb_ctx->devices[i]->services[j]->characteristics[k]->char_path);
                if (lb_ctx->devices[i]->services[j]->characteristics[k]->uuid != NULL)
                    free((char*) lb_ctx->devices[i]->services[j]->characteristics[k]->uuid);
                if (lb_ctx->devices[i]->services[j]->characteristics[k] != NULL)
                    free(lb_ctx->devices[i]->services[j]->characteristics[k]);
            }
            if (lb_ctx->devices[i]->services[j]->service_path != NULL)
                free((char*) lb_ctx->devices[i]->services[j]->service_path);
            if (lb_ctx->devices[i]->services[j]->uuid != NULL)
                free((char*) lb_ctx->devices[i]->services[j]->uuid);
            if (lb_ctx->devices[i]->services[j]->characteristics != NULL)
                free(lb_ctx->devices[i]->services[j]->characteristics);
            if (lb_ctx->devices[i]->services[j] != NULL)
                free(lb_ctx->devices[i]->services[j]);
        }
        if (lb_ctx->devices[i]->address != NULL)
            free((char*) lb_ctx->devices[i]->address);
        if (lb_ctx->devices[i]->device_path != NULL)
            free((char*) lb_ctx->devices[i]->device_path);
        if (lb_ctx->devices[i]->name != NULL)
            free((char*) lb_ctx->devices[i]->name);
        if (lb_ctx->devices[i]->services != NULL)
            free(lb_ctx->devices[i]->services);
        if (lb_ctx->devices[i] != NULL)
            free(lb_ctx->devices[i]);
    }
    if (lb_ctx->devices != NULL)
        free(lb_ctx->devices);
    free(lb_ctx);

    lb_ctx = NULL;
    return LB_SUCCESS;
}

lb_result_t
lb_init()
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    if (lb_context_new() != LB_SUCCESS)
        return LB_ERROR_INVALID_CONTEXT;

    return LB_SUCCESS;
}

lb_result_t
lb_destroy()
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int i, r;

    pthread_cancel(event_thread);
    pthread_join(event_thread, NULL);

    r = sd_event_exit(event, SIGINT);
    if (r < 0) {
        syslog(LOG_ERR, "%s: failed to stop event loop", __FUNCTION__);
    }
    sd_event_unref(event);

    if (events_matches_array != NULL) {
        for (i = 0; i < event_arr_size; i++) {
            free(events_matches_array[i]);
        }
        free(events_matches_array);
    }

    lb_context_free();

    return LB_SUCCESS;
}

lb_result_t
lb_get_bl_devices(int seconds)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    const char** objects;
    int i = 0, r = 0;

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    if (lb_ctx->devices != NULL) {
        free(lb_ctx->devices);
    }

    objects = (const char**) calloc(MAX_OBJECTS, MAX_OBJECTS * sizeof(const char*));
    if (objects == NULL) {
        syslog(LOG_ERR, "%s: Error allocating memory for objects array", __FUNCTION__);
        return -LB_ERROR_MEMEORY_ALLOCATION;
    }

    r = _scan_devices(seconds);
    if (r < 0) {
        syslog(LOG_ERR, "%s: Error getting root objects", __FUNCTION__);
        return -LB_ERROR_UNSPECIFIED;
    }

    r = _get_root_objects(objects);
    if (r < 0) {
        syslog(LOG_ERR, "%s: Error getting root objects", __FUNCTION__);
        return -LB_ERROR_UNSPECIFIED;
    }

    while (objects[i] != NULL) {
        if (_is_bl_device(objects[i])) {
            _add_new_device(objects[i]);
        }
        free((char*) objects[i]);
        i++;
    }

    free(objects);

    return LB_SUCCESS;
}

lb_result_t
lb_connect_device(lb_bl_device* dev)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int r;
    sd_bus_error error = SD_BUS_ERROR_NULL;

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    if (dev == NULL) {
        syslog(LOG_ERR, "%s: bl_device is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (!_is_bus_connected(lb_ctx)) {
        syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
        sd_bus_error_free(&error);
        return -LB_ERROR_INVALID_BUS;
    }

    r = sd_bus_call_method(lb_ctx->bus, BLUEZ_DEST, dev->device_path, BLUEZ_DEVICE, "Connect",
                           &error, NULL, NULL);

    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_call_method Connect on device %s failed with error: %s",
               __FUNCTION__, dev->device_path, error.message);
        sd_bus_error_free(&error);
        return -LB_ERROR_SD_BUS_CALL_FAIL;
    }

    sd_bus_error_free(&error);
    return LB_SUCCESS;
}

lb_result_t
lb_disconnect_device(lb_bl_device* dev)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int r;
    sd_bus_error error = SD_BUS_ERROR_NULL;

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    if (dev == NULL) {
        syslog(LOG_ERR, "%s: bl_device is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (!_is_bus_connected(lb_ctx)) {
        syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
        sd_bus_error_free(&error);
        return -LB_ERROR_INVALID_BUS;
    }

    r = sd_bus_call_method(lb_ctx->bus, BLUEZ_DEST, dev->device_path, BLUEZ_DEVICE, "Disconnect",
                           &error, NULL, NULL);

    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_call_method Disconnect on device: %s failed with error: %s",
               __FUNCTION__, dev->device_path, error.message);
        sd_bus_error_free(&error);
        return -LB_ERROR_SD_BUS_CALL_FAIL;
    }

    sd_bus_error_free(&error);
    return LB_SUCCESS;
}

lb_result_t
lb_pair_device(lb_bl_device* dev)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int r;
    sd_bus_error error = SD_BUS_ERROR_NULL;

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    if (dev == NULL) {
        syslog(LOG_ERR, "%s: bl_device is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (!_is_bus_connected(lb_ctx)) {
        syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
        sd_bus_error_free(&error);
        return -LB_ERROR_INVALID_BUS;
    }

    r = sd_bus_call_method(lb_ctx->bus, BLUEZ_DEST, dev->device_path, BLUEZ_DEVICE, "Pair", &error, NULL, NULL);

    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_call_method Pair on device %s failed with error: %s",
               __FUNCTION__, dev->device_path, error.message);
        sd_bus_error_free(&error);
        return -LB_ERROR_SD_BUS_CALL_FAIL;
    }

    sd_bus_error_free(&error);
    return LB_SUCCESS;
}

lb_result_t
lb_unpair_device(lb_bl_device* dev)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int r;
    sd_bus_error error = SD_BUS_ERROR_NULL;

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    if (dev == NULL) {
        syslog(LOG_ERR, "%s: bl_device is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (!_is_bus_connected(lb_ctx)) {
        syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
        sd_bus_error_free(&error);
        return -LB_ERROR_INVALID_BUS;
    }

    r = sd_bus_call_method(lb_ctx->bus, BLUEZ_DEST, dev->device_path, BLUEZ_DEVICE, "CancelPairing",
                           &error, NULL, NULL);

    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_call_method CancelPairing on device %s failed with error: %s",
               __FUNCTION__, dev->device_path, error.message);
        sd_bus_error_free(&error);
        return -LB_ERROR_SD_BUS_CALL_FAIL;
    }

    sd_bus_error_free(&error);
    return LB_SUCCESS;
}

lb_result_t
lb_get_ble_device_services(lb_bl_device* dev)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int i = 0, r = 0;
    sd_bus_error error = SD_BUS_ERROR_NULL;

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    if (dev == NULL) {
        syslog(LOG_ERR, "%s: bl_device is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (!_is_bl_device(dev->device_path)) {
        syslog(LOG_ERR, "%s: device %s not a bl device", __FUNCTION__, dev->device_path);
        sd_bus_error_free(&error);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (!_is_device_paired(dev->device_path)) {
        r = lb_pair_device(dev);
        if (r < 0) {
            syslog(LOG_ERR, "%s: error pairing device", __FUNCTION__);
            sd_bus_error_free(&error);
            return -LB_ERROR_UNSPECIFIED;
        }
    }

    if (dev->services != NULL) {
        free(dev->services);
    }

    const char** objects = (const char**) calloc(MAX_OBJECTS, MAX_OBJECTS * sizeof(const char*));
    if (objects == NULL) {
        syslog(LOG_ERR, "%s: Error allocating memory for objects array", __FUNCTION__);
        sd_bus_error_free(&error);
        return -LB_ERROR_MEMEORY_ALLOCATION;
    }

    r = _get_root_objects(objects);
    if (r < 0) {
        syslog(LOG_ERR, "%s: Error getting root objects", __FUNCTION__);
        if (objects != NULL)
            free(objects);
        sd_bus_error_free(&error);
        return -LB_ERROR_UNSPECIFIED;
    }

    while (objects[i] != NULL) {
        if (strstr(objects[i], dev->device_path) && _is_ble_service(objects[i])) {
            const char* service_path = objects[i];
            r = _add_new_service(dev, service_path);
            if (r < 0) {
                syslog(LOG_ERR, "%s: Error adding ble service", __FUNCTION__);
                continue;
            }
            int j = 0;
            while (objects[j] != NULL) {
                if (strstr(objects[j], service_path) && _is_ble_characteristic(objects[j])) {
                    lb_ble_service* new_service = NULL;
                    r = lb_get_ble_service_by_service_path(dev, service_path, &new_service);
                    if (r < 0) {
                        syslog(LOG_ERR, "%s: Error getting ble service", __FUNCTION__);
                        continue;
                    }
                    _add_new_characteristic(new_service, objects[j]);
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

    i = 0;
    while (objects[i] != NULL) {
        free((char*) objects[i]);
        i++;
    }
    free(objects);

    sd_bus_error_free(&error);

    return LB_SUCCESS;
}

lb_result_t
lb_get_ble_characteristic_by_characteristic_path(lb_bl_device* dev,
                                                 const char* characteristic_path,
                                                 lb_ble_char** ble_characteristic_ret)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int i, j;

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    if (dev == NULL) {
        syslog(LOG_ERR, "%s: bl_device is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (characteristic_path == NULL) {
        syslog(LOG_ERR, "%s: characteristic_path is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    for (i = 0; i < dev->services_size; i++) {
        for (j = 0; dev->services[i]->characteristics_size; j++) {
            if (strncmp(characteristic_path, dev->services[i]->characteristics[j]->char_path,
                        strlen(characteristic_path)) == 0) {
                *ble_characteristic_ret = dev->services[i]->characteristics[j];
                return LB_SUCCESS;
            }
        }
    }
    return -LB_ERROR_UNSPECIFIED;
}

lb_result_t
lb_get_ble_characteristic_by_uuid(lb_bl_device* dev, const char* uuid, lb_ble_char** ble_characteristic_ret)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int i, j;

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    if (dev == NULL) {
        syslog(LOG_ERR, "%s: bl_device is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (uuid == NULL) {
        syslog(LOG_ERR, "%s: uuid is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (!_is_ble_device(dev->device_path)) {
        syslog(LOG_ERR, "%s: not a ble device", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    for (i = 0; i < dev->services_size; i++) {
        for (j = 0; dev->services_size; j++) {
            if (strncmp(uuid, dev->services[i]->characteristics[j]->uuid, strlen(uuid)) == 0) {
                *ble_characteristic_ret = dev->services[i]->characteristics[j];
                return LB_SUCCESS;
            }
        }
    }
    return -LB_ERROR_UNSPECIFIED;
}

lb_result_t
lb_get_ble_service_by_service_path(lb_bl_device* dev, const char* service_path, lb_ble_service** ble_service_ret)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int i;

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    if (dev == NULL) {
        syslog(LOG_ERR, "%s: bl_device is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (service_path == NULL) {
        syslog(LOG_ERR, "%s: service_path is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    for (i = 0; i < dev->services_size; i++) {
        if (strncmp(service_path, dev->services[i]->service_path, strlen(service_path)) == 0) {
            *ble_service_ret = dev->services[i];
            return LB_SUCCESS;
        }
    }
    return -LB_ERROR_UNSPECIFIED;
}

lb_result_t
lb_get_ble_service_by_uuid(lb_bl_device* dev, const char* uuid, lb_ble_service** ble_service_ret)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int i;

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    if (dev == NULL) {
        syslog(LOG_ERR, "%s: bl_device is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (uuid == NULL) {
        syslog(LOG_ERR, "%s: uuid is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (!_is_ble_device(dev->device_path)) {
        syslog(LOG_ERR, "%s: not a ble device", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    for (i = 0; i < dev->services_size; i++) {
        if (strncmp(uuid, dev->services[i]->uuid, strlen(uuid)) == 0) {
            *ble_service_ret = dev->services[i];
            return LB_SUCCESS;
        }
    }
    return -LB_ERROR_UNSPECIFIED;
}

lb_result_t
lb_get_device_by_device_path(const char* device_path, lb_bl_device** bl_device_ret)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int i;

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    if (device_path == NULL) {
        syslog(LOG_ERR, "%s: device_path is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    for (i = 0; i < lb_ctx->devices_size; i++) {
        if (strncmp(device_path, lb_ctx->devices[i]->device_path, strlen(device_path)) == 0) {
            *bl_device_ret = lb_ctx->devices[i];
            return LB_SUCCESS;
        }
    }
    return -LB_ERROR_UNSPECIFIED;
}

lb_result_t
lb_get_device_by_device_name(const char* name, lb_bl_device** bl_device_ret)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int i;

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    if (name == NULL) {
        syslog(LOG_ERR, "%s: name is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    for (i = 0; i < lb_ctx->devices_size; i++) {
        if (strncmp(name, lb_ctx->devices[i]->name, strlen(name)) == 0) {
            *bl_device_ret = lb_ctx->devices[i];
            return LB_SUCCESS;
        }
    }
    return -LB_ERROR_UNSPECIFIED;
}

lb_result_t
lb_get_device_by_device_address(const char* address, lb_bl_device** bl_device_ret)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int i;

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    if (address == NULL) {
        syslog(LOG_ERR, "%s: address is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    for (i = 0; i < lb_ctx->devices_size; i++) {
        if (strncmp(address, lb_ctx->devices[i]->address, strlen(address)) == 0) {
            *bl_device_ret = lb_ctx->devices[i];
            return LB_SUCCESS;
        }
    }
    return -LB_ERROR_UNSPECIFIED;
}

lb_result_t
lb_write_to_characteristic(lb_bl_device* dev, const char* uuid, int size, uint8_t* value)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int r;
    sd_bus_message* func_call = NULL;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    lb_ble_char* characteristics = NULL;

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    if (dev == NULL) {
        syslog(LOG_ERR, "%s: bl_device is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (uuid == NULL) {
        syslog(LOG_ERR, "%s: uuid is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (!_is_bus_connected(lb_ctx)) {
        syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
        sd_bus_error_free(&error);
        return -LB_ERROR_INVALID_BUS;
    }

    r = lb_get_ble_characteristic_by_uuid(dev, uuid, &characteristics);
    if (r < 0) {
        syslog(LOG_ERR, "%s: Failed to get characteristic", __FUNCTION__);
        sd_bus_error_free(&error);
        return -LB_ERROR_UNSPECIFIED;
    }

    r = sd_bus_message_new_method_call(lb_ctx->bus, &func_call, BLUEZ_DEST, characteristics->char_path,
                                       BLUEZ_GATT_CHARACTERISTICS, "WriteValue");
    if (r < 0) {
        syslog(LOG_ERR, "%s: Failed to create message call", __FUNCTION__);
        sd_bus_error_free(&error);
        sd_bus_message_unref(func_call);
        return -LB_ERROR_UNSPECIFIED;
    }

    r = sd_bus_message_append_array(func_call, 'y', value, size);
    if (r < 0) {
        syslog(LOG_ERR, "%s: Failed to append array to message call", __FUNCTION__);
        sd_bus_error_free(&error);
        sd_bus_message_unref(func_call);
        return -LB_ERROR_UNSPECIFIED;
    }

    r = sd_bus_message_append(func_call, "a{sv}", 0, NULL);
    if (r < 0) {
        syslog(LOG_ERR, "%s: Failed to append a{sv} to message call", __FUNCTION__);
        sd_bus_error_free(&error);
        sd_bus_message_unref(func_call);
        return -LB_ERROR_UNSPECIFIED;
    }

    r = sd_bus_call(lb_ctx->bus, func_call, 0, &error, NULL);
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_call WriteValue on device %s failed with error: %s",
               __FUNCTION__, characteristics->char_path, error.message);
        sd_bus_error_free(&error);
        sd_bus_message_unref(func_call);
        return -LB_ERROR_SD_BUS_CALL_FAIL;
    }
    r = sd_bus_process(lb_ctx->bus, NULL);
    if (r < 0) {
        syslog(LOG_ERR, "Failed to process bus: %s\n", strerror(-r));
        sd_bus_error_free(&error);
        sd_bus_message_unref(func_call);
        return -LB_ERROR_SD_BUS_CALL_FAIL;
    }

    sd_bus_error_free(&error);
    sd_bus_message_unref(func_call);
    return LB_SUCCESS;
}

lb_result_t
lb_read_from_characteristic(lb_bl_device* dev, const char* uuid, size_t* size, uint8_t** result)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int r;
    sd_bus_message* reply = NULL;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    lb_ble_char* characteristics = NULL;

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    if (dev == NULL) {
        syslog(LOG_ERR, "%s: bl_device is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (uuid == NULL) {
        syslog(LOG_ERR, "%s: uuid is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (!_is_bus_connected(lb_ctx)) {
        syslog(LOG_ERR, "%s: Bus is not opened", __FUNCTION__);
        sd_bus_error_free(&error);
        return -LB_ERROR_INVALID_BUS;
    }

    if (!_is_ble_device(dev->device_path)) {
        syslog(LOG_ERR, "%s: not a ble device", __FUNCTION__);
        sd_bus_error_free(&error);
        return -LB_ERROR_INVALID_DEVICE;
    }

    r = lb_get_ble_characteristic_by_uuid(dev, uuid, &characteristics);
    if (r < 0) {
        syslog(LOG_ERR, "%s: Failed to get characteristic", __FUNCTION__);
        sd_bus_error_free(&error);
        return -LB_ERROR_UNSPECIFIED;
    }

    r = sd_bus_call_method(lb_ctx->bus, BLUEZ_DEST, characteristics->char_path,
                           BLUEZ_GATT_CHARACTERISTICS, "ReadValue", &error, &reply, "a{sv}", NULL);
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_call_method ReadValue on device %s failed with error: %s",
               __FUNCTION__, characteristics->char_path, error.message);
        sd_bus_error_free(&error);
        sd_bus_message_unref(reply);
        return -LB_ERROR_SD_BUS_CALL_FAIL;
    }

    r = sd_bus_message_read_array(reply, 'y', (const void**) result, size);
    if (r < 0) {
        syslog(LOG_ERR, "%s: Failed to read byte array message", __FUNCTION__);
        sd_bus_error_free(&error);
        sd_bus_message_unref(reply);
        return -LB_ERROR_UNSPECIFIED;
    }

    sd_bus_error_free(&error);
    sd_bus_message_unref(reply);
    return LB_SUCCESS;
}

lb_result_t
lb_register_characteristic_read_event(lb_bl_device* dev,
                                      const char* uuid,
                                      sd_bus_message_handler_t callback,
                                      void* userdata)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int r;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    lb_ble_char* ble_char_new = NULL;
    char match[68];

    if (lb_ctx == NULL) {
        syslog(LOG_ERR, "%s: lb_ctx is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    if (dev == NULL) {
        syslog(LOG_ERR, "%s: bl_device is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (uuid == NULL) {
        syslog(LOG_ERR, "%s: uuid is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    if (callback == NULL) {
        syslog(LOG_ERR, "%s: callback is null", __FUNCTION__);
        return -LB_ERROR_INVALID_DEVICE;
    }

    r = lb_get_ble_characteristic_by_uuid(dev, uuid, &ble_char_new);
    if (r < 0) {
        syslog(LOG_ERR, "%s: could find characteristic: %s", __FUNCTION__, uuid);
        sd_bus_error_free(&error);
        return -LB_ERROR_UNSPECIFIED;
    }

    r = sd_bus_call_method(lb_ctx->bus, BLUEZ_DEST, ble_char_new->char_path,
                           BLUEZ_GATT_CHARACTERISTICS, "StartNotify", &error, NULL, NULL);
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_call_method StartNotify on device %s failed with error: %s",
               __FUNCTION__, ble_char_new->char_path, error.message);
        sd_bus_error_free(&error);
        return -LB_ERROR_SD_BUS_CALL_FAIL;
    }

    snprintf(match, 67, "path='%s'", ble_char_new->char_path);

    int current_index = event_arr_size;
    if (event_arr_size == 0 || events_matches_array == NULL) {
        events_matches_array = (event_matches_callbacks**) malloc(sizeof(event_matches_callbacks*));
        if (events_matches_array == NULL) {
            syslog(LOG_ERR, "%s: Error allocating memory for events_matches_array", __FUNCTION__);
            return -LB_ERROR_MEMEORY_ALLOCATION;
        }
        event_arr_size++;
    } else {
        event_arr_size++;
        events_matches_array =
        realloc(events_matches_array, event_arr_size * sizeof(event_matches_callbacks*));
        if (events_matches_array == NULL) {
            syslog(LOG_ERR, "%s: Error reallocating memory for events_matches_array", __FUNCTION__);
            return -LB_ERROR_MEMEORY_ALLOCATION;
        }
    }

    event_matches_callbacks* new_event_pair =
    (event_matches_callbacks*) malloc(sizeof(event_matches_callbacks));
    if (new_event_pair == NULL) {
        syslog(LOG_ERR, "%s: Error reallocating memory for events_matches_array", __FUNCTION__);
        return -LB_ERROR_MEMEORY_ALLOCATION;
    }
    new_event_pair->event = match;
    new_event_pair->callback = &callback;
    new_event_pair->userdata = userdata;
    events_matches_array[current_index] = new_event_pair;

    pthread_create(&event_thread, NULL, _run_event_loop, &(lb_ctx->bus));
    // wait for thread to start
    sleep(2);

    sd_bus_error_free(&error);
    return LB_SUCCESS;
}

lb_result_t
lb_parse_uart_service_message(sd_bus_message* message, const void** result, size_t* size)
{
#ifdef DEBUG
    printf("Method Called: %s\n", __FUNCTION__);
#endif
    int r;

    if (message == NULL) {
        syslog(LOG_ERR, "%s: message is null", __FUNCTION__);
        return -LB_ERROR_INVALID_CONTEXT;
    }

    r = sd_bus_message_skip(message, "s");
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_message_skip failed with error: %s", __FUNCTION__, strerror(-r));
        return -LB_ERROR_UNSPECIFIED;
    }

    r = sd_bus_message_enter_container(message, 'a', "{sv}");
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_message_enter_container {sv} failed with error: %s",
               __FUNCTION__, strerror(-r));
        return -LB_ERROR_UNSPECIFIED;
    }


    while ((r = sd_bus_message_enter_container(message, 'e', "sv")) > 0) {
        r = sd_bus_message_skip(message, "s");
        if (r < 0) {
            syslog(LOG_ERR, "%s: sd_bus_message_skip failed with error: %s", __FUNCTION__, strerror(-r));
            return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_message_enter_container(message, 'v', "ay");
        if (r < 0) {
            syslog(LOG_ERR, "%s: sd_bus_message_enter_container v failed with error: %s",
                   __FUNCTION__, strerror(-r));
            return -LB_ERROR_UNSPECIFIED;
        }


        r = sd_bus_message_read_array(message, 'y', result, size);
        if (r < 0) {
            syslog(LOG_ERR, "%s: Failed to read byte array message with error: %s", __FUNCTION__,
                   strerror(-r));
            return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_message_exit_container(message);
        if (r < 0) {
            syslog(LOG_ERR, "%s: sd_bus_message_exit_container v failed with error: %s",
                   __FUNCTION__, strerror(-r));
            return -LB_ERROR_UNSPECIFIED;
        }

        r = sd_bus_message_exit_container(message);
        if (r < 0) {
            syslog(LOG_ERR, "%s: sd_bus_message_exit_container sv failed with error: %s",
                   __FUNCTION__, strerror(-r));
            return -LB_ERROR_UNSPECIFIED;
        }
    }

    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_message_enter_container sv failed with error: %s", __FUNCTION__,
               strerror(-r));
        return -LB_ERROR_UNSPECIFIED;
    }

    r = sd_bus_message_exit_container(message);
    if (r < 0) {
        syslog(LOG_ERR, "%s: sd_bus_message_exit_container {sv} failed with error: %s",
               __FUNCTION__, strerror(-r));
        return -LB_ERROR_UNSPECIFIED;
    }

    return LB_SUCCESS;
}
