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
                fprintf(stderr, "Error copying address to new_addresss\n");
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
                fprintf(stderr, "Failed to issue method call: %s on %s\n", error.message, device_path);
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
                fprintf(stderr, "Failed to issue method call: %s on %s\n", error.message, device_path);
                sd_bus_error_free(&error);
                return NULL;
        }

        return (const char *) address;
}

const char*
get_device_path_uuid(lb_context *lb_ctx, const char *service_path, const char *interface)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        sd_bus_error error = SD_BUS_ERROR_NULL;
        int r;
        char *name;

        r = sd_bus_get_property_string(lb_ctx->bus, "org.bluez", service_path, interface, "UUID", &error, &name);
        if(r < 0) {
                fprintf(stderr, "Failed to issue method call: %s on %s\n", error.message, service_path);
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
                fprintf(stderr, "Bus is not opened\n");
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
                fprintf(stderr, "Failed to issue method call: %s\n", error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);;
                return false;
        }

        r = sd_bus_message_read_basic(m, 's', &introspect_xml);
        if(r < 0) {
                fprintf(stderr, "sd_bus_message_read_basic: %s\n", strerror(-r));
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

int
get_root_objects(lb_context *lb_ctx, const char **objects)
{
        if (DEBUG > 1) printf("Method Called: %s\n", __FUNCTION__);
        int r = 0, i = 0;
        const char *device_path;
        sd_bus_error error = SD_BUS_ERROR_NULL;
        sd_bus_message *m = NULL;

        if(!is_bus_connected(lb_ctx)) {
                fprintf(stderr, "Bus is not opened\n");
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
                fprintf(stderr, "Failed to issue method call: %s\n", error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        /* Parse the response message */
        //"a{oa{sa{sv}}}"
        r = sd_bus_message_enter_container(m, 'a', "{oa{sa{sv}}}");
        if(r < 0) {
                fprintf(stderr, "sd_bus_message_enter_container {oa{sa{sv}}}: %s\n", strerror(-r));
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        while((r = sd_bus_message_enter_container(m, 'e', "oa{sa{sv}}")) > 0) {
                r = sd_bus_message_read_basic(m, 'o', &device_path);
                if(r < 0) {
                        fprintf(stderr, "sd_bus_message_read_basic: %s\n", strerror(-r));
                                sd_bus_error_free(&error);
        sd_bus_message_unref(m);;
                }
                else {
                        char *restrict new_device_path = malloc(strlen(device_path) + 1);
                        if(new_device_path == NULL) {
                                fprintf(stderr, "Error allocating memory for object name\n");
                                sd_bus_error_free(&error);
                                sd_bus_message_unref(m);
                                return EXIT_FAILURE;
                        }
                        objects[i] = strcpy(new_device_path, device_path);
                        i++;
                }

                r = sd_bus_message_skip(m, "a{sa{sv}}");
                if(r < 0) {
                        fprintf(stderr, "sd_bus_message_skip: %s\n", strerror(-r));
                        sd_bus_error_free(&error);
                        sd_bus_message_unref(m);
                        return EXIT_FAILURE;
                }

                r = sd_bus_message_exit_container(m);
                if(r < 0) {
                        fprintf(stderr, "sd_bus_message_exit_container oa{sa{sv}}: %s\n", strerror(-r));
                        sd_bus_error_free(&error);
                        sd_bus_message_unref(m);
                        return EXIT_FAILURE;
                }
        }

        if(r < 0) {
                fprintf(stderr, "sd_bus_message_enter_container oa{sa{sv}}: %s\n", strerror(-r));
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_message_exit_container(m);
        if(r < 0) {
                fprintf(stderr, "sd_bus_message_exit_container {oa{sa{sv}}}: %s\n", strerror(-r));
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
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
                        fprintf(stderr, "Error reallocating memory for devices\n");
                        return EXIT_FAILURE;
                }
        }

        bl_device *new_device = malloc(sizeof(bl_device));

        new_device->device_path = device_path;
        const char *name = get_device_name(lb_ctx, device_path);
        if(name == NULL) {
                fprintf(stderr, "Error couldn't find device name\n");
                new_device->name = "null";
        }
        else {
                new_device->name = name;
        }
        const char *address = get_device_address(lb_ctx, device_path);
        if(address == NULL) {
                fprintf(stderr, "Error couldn't find device address\n");
                new_device->address = "null";
        }
        else {
                new_device->address = address;
        }

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
                fprintf(stderr, "Bus is not opened\n");
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_call_method(lb_ctx->bus, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", "StartDiscovery", &error, &m,
        NULL);
        if(r < 0) {
                fprintf(stderr, "Failed to issue method call: %s\n", error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sleep(seconds);

        r = sd_bus_call_method(lb_ctx->bus, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", "StopDiscovery", &error, &m,
        NULL);

        if(r < 0) {
                fprintf(stderr, "Failed to issue method call: %s\n", error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
lb_open_system_bus(lb_context *lb_ctx)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        /* Connect to the system bus */
        r = sd_bus_open_system(&lb_ctx->bus);
        if(r < 0) {
                fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        }
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
                fprintf(stderr, "Error allocating memory for objects array\n");
                return EXIT_FAILURE;
        }

        scan_devices(lb_ctx, seconds);

        get_root_objects(lb_ctx, objects);

        //if (r < 0) {
        //      fprintf(stderr, "Error getting root objects\n");
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
lb_connect_device(lb_context *lb_ctx, const char  *address)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(!is_bus_connected(lb_ctx)) {
                fprintf(stderr, "Bus is not opened\n");
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               "org.bluez",
                               address,
                               "org.bluez.Device1",
                               "Connect",
                               &error,
                               &m,
                               NULL);

        if(r < 0) {
                fprintf(stderr, "Failed to issue method call: %s\n", error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
lb_disconnect_device(lb_context *lb_ctx, const char  *address)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(!is_bus_connected(lb_ctx)) {
                fprintf(stderr, "Bus is not opened\n");
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               "org.bluez",
                               address,
                               "org.bluez.Device1",
                               "Disconnect",
                               &error,
                               &m,
                               NULL);

        if(r < 0) {
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
lb_pair_device(lb_context *lb_ctx, const char  *address)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(!is_bus_connected(lb_ctx)) {
                fprintf(stderr, "Bus is not opened\n");
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               "org.bluez",
                               address,
                               "org.bluez.Device1",
                               "Pair",
                               &error,
                               &m,
                               NULL);

        if(r < 0) {
                fprintf(stderr, "Failed to issue method call: %s\n", error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
lb_unpair_device(lb_context *lb_ctx, const char  *address)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(!is_bus_connected(lb_ctx)) {
                fprintf(stderr, "Bus is not opened\n");
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               "org.bluez",
                               address,
                               "org.bluez.Device1",
                               "CancelPairing",
                               &error,
                               &m,
                               NULL);

        if(r < 0) {
                fprintf(stderr, "Failed to issue method call: %s\n", error.message);
                sd_bus_error_free(&error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        printf("Method Called: %s\n", "CancelPairing");

        sd_bus_error_free(&error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
lb_get_ble_device_services(lb_context *lb_ctx, const char* device_path, ble_service **services)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        const char *point;
        int i = 0, r = 0;
        sd_bus_error error = SD_BUS_ERROR_NULL;

        if(lb_ctx->devices != NULL) {
                free(lb_ctx->devices);
        }

        const char **objects = malloc(MAX_OBJECTS  *sizeof(const char *));
        if(objects == NULL) {
                fprintf(stderr, "Error allocating memory for objects array\n");
                return EXIT_FAILURE;
        }

        get_root_objects(lb_ctx, objects);

        //if (r < 0) {
        //      fprintf(stderr, "Error getting root objects\n");
        //      free(objects);
        //      return -1;
        //}

        while(objects[i] != NULL) {
                if(strstr(objects[i], device_path) && is_ble_service(lb_ctx, objects[i])) {
                        printf("ble_service: %s\nuuid: %s\n", objects[i], get_device_path_uuid(lb_ctx, objects[i], "org.bluez.GattService1"));
                }
                if(strstr(objects[i], device_path) && is_ble_characteristic(lb_ctx, objects[i])) {
                        printf("ble_characteristic: %s\nuuid: %s\n", objects[i], get_device_path_uuid(lb_ctx, objects[i], "org.bluez.GattCharacteristic1"));
                }
                i++;
        }

        return EXIT_SUCCESS;
}

int
lb_get_device_by_device_path(lb_context *lb_ctx, const char *device_path, bl_device **bl_device_pointer)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i;
        for(i = 0; i < lb_ctx->devices_size; i++) {
                if (strncmp(device_path, lb_ctx->devices[i]->device_path, strlen(device_path)) == 0) {
                        *bl_device_pointer = lb_ctx->devices[i];
                        return EXIT_SUCCESS;
                }
        }
        return EXIT_FAILURE;
}

int
lb_get_device_by_device_name(lb_context *lb_ctx, const char *name, bl_device **bl_device_pointer)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i;
        for(i = 0; i < lb_ctx->devices_size; i++) {
                if (strncmp(name, lb_ctx->devices[i]->name, strlen(name)) == 0) {
                        *bl_device_pointer = lb_ctx->devices[i];
                        return EXIT_SUCCESS;
                }
        }
        return EXIT_FAILURE;
}

int
lb_get_device_by_device_address(lb_context *lb_ctx, const char *address, bl_device **bl_device_pointer)
{
        if (DEBUG > 0) printf("Method Called: %s\n", __FUNCTION__);
        int i;
        for(i = 0; i < lb_ctx->devices_size; i++) {
                if (strncmp(address, lb_ctx->devices[i]->address, strlen(address)) == 0) {
                        *bl_device_pointer = lb_ctx->devices[i];
                        return EXIT_SUCCESS;
                }
        }
        return EXIT_FAILURE;
}