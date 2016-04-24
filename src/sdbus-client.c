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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <systemd/sd-bus.h>

#define MAX_LEN 256
#define MAX_OBJECTS 256

typedef struct bl_device
{
        const char *object_path;
        const char *address;
        const char *name;
} bl_device;


typedef struct lb_context
{
        sd_bus *bus;
        bl_device **devices;
        int devices_size;
}lb_context;


const char*
convert_object_path_to_address(const char *address)
{
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
        sd_bus_error *error = NULL;
        printf("Method Called: %s\n", __FUNCTION__);
        int r;
        char *name;

        r = sd_bus_get_property_string(lb_ctx->bus, "org.bluez", device_path, "org.bluez.Device1", "Name", error, &name);
        if(r < 0) {
                fprintf(stderr, "Failed to issue method call: %s on %s\n", error->message, device_path);
                sd_bus_error_free(error);
                return NULL;
        }

        return (const char *) name;
}

const char*
get_object_path_by_address(lb_context *lb_ctx, const char *object_path)
{
        int i;
        for(i = 0; i < lb_ctx->devices_size; i++) {
                if (strncmp(object_path, lb_ctx->devices[i]->object_path, strlen(object_path)) == 0) {
                        return lb_ctx->devices[i]->address;
                }
        }
        return NULL;
}

bool
is_bl_device(lb_context *lb_ctx, const char *object_path)
{
        printf("Method Called: %s\n", __FUNCTION__);
        sd_bus_error *error = NULL;
        sd_bus_message *m = NULL;

        int r;
        const char *introspect_xml;

        if(!is_bus_connected(lb_ctx)) {
                fprintf(stderr, "Bus is not opened\n");
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               "org.bluez",
                               object_path,
                               "org.freedesktop.DBus.Introspectable",
                               "Introspect",
                               error,
                               &m,
                               NULL);
        if(r < 0) {
                fprintf(stderr, "Failed to issue method call: %s\n", error->message);
                sd_bus_error_free(error);
                sd_bus_message_unref(m);;
                return false;
        }

        r = sd_bus_message_read_basic(m, 's', &introspect_xml);
        if(r < 0) {
                fprintf(stderr, "sd_bus_message_read_basic: %s\n", strerror(-r));
                sd_bus_error_free(error);
                sd_bus_message_unref(m);
                return false;
        }

        sd_bus_error_free(error);
        sd_bus_message_unref(m);

        return (strstr(introspect_xml, "org.bluez.Device1") != NULL) ? true : false;
}

int
add_new_device(lb_context *lb_ctx, const char *device_path)
{
        printf("Method Called: %s\n", __FUNCTION__);

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

        new_device->object_path = device_path;
        const char *name = get_device_name(lb_ctx, device_path);
        if(name == NULL) {
                fprintf(stderr, "Error couldn't find device name\n");
                new_device->name = "null";
        }
        else {
                new_device->name = name;
        }
        new_device->address = convert_object_path_to_address(device_path);
        lb_ctx->devices[current_index] = new_device;

        return EXIT_SUCCESS;
}

int
get_root_objects(lb_context *lb_ctx, const char* *objects)
{
        printf("Method Called: %s\n", __FUNCTION__);
        int r = 0, i = 0;
        const char *object_path;
        sd_bus_error *error = NULL;
        sd_bus_message *m = NULL;

        if(!is_bus_connected(lb_ctx)) {
                fprintf(stderr, "Bus is not opened\n");
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               "org.bluez",
                               "/",
                               "org.freedesktop.DBus.ObjectManager",
                               "GetManagedObjects",
                               error,
                               &m,
                               NULL);
        if(r < 0) {
                fprintf(stderr, "Failed to issue method call: %s\n", error->message);
                sd_bus_error_free(error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        /* Parse the response message */
        //"a{oa{sa{sv}}}"
        r = sd_bus_message_enter_container(m, 'a', "{oa{sa{sv}}}");
        if(r < 0) {
                fprintf(stderr, "sd_bus_message_enter_container {oa{sa{sv}}}: %s\n", strerror(-r));
                sd_bus_error_free(error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        while((r = sd_bus_message_enter_container(m, 'e', "oa{sa{sv}}")) > 0) {
                r = sd_bus_message_read_basic(m, 'o', &object_path);
                if(r < 0) {
                        fprintf(stderr, "sd_bus_message_read_basic: %s\n", strerror(-r));
                                sd_bus_error_free(error);
        sd_bus_message_unref(m);;
                }
                else {
                        char *restrict new_object_path = malloc(strlen(object_path) + 1);
                        if(new_object_path == NULL) {
                                fprintf(stderr, "Error allocating memory for object name\n");
                                sd_bus_error_free(error);
                                sd_bus_message_unref(m);
                                return EXIT_FAILURE;
                        }
                        objects[i] = strcpy(new_object_path, object_path);
                        i++;
                }

                r = sd_bus_message_skip(m, "a{sa{sv}}");
                if(r < 0) {
                        fprintf(stderr, "sd_bus_message_skip: %s\n", strerror(-r));
                        sd_bus_error_free(error);
                        sd_bus_message_unref(m);
                        return EXIT_FAILURE;
                }

                r = sd_bus_message_exit_container(m);
                if(r < 0) {
                        fprintf(stderr, "sd_bus_message_exit_container oa{sa{sv}}: %s\n", strerror(-r));
                        sd_bus_error_free(error);
                        sd_bus_message_unref(m);
                        return EXIT_FAILURE;
                }
        }

        if(r < 0) {
                fprintf(stderr, "sd_bus_message_enter_container oa{sa{sv}}: %s\n", strerror(-r));
                sd_bus_error_free(error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_message_exit_container(m);
        if(r < 0) {
                fprintf(stderr, "sd_bus_message_exit_container {oa{sa{sv}}}: %s\n", strerror(-r));
                sd_bus_error_free(error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sd_bus_error_free(error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
scan_devices(lb_context *lb_ctx, int seconds)
{
        printf("Method Called: %s\n", __FUNCTION__);
        int r = 0;
        sd_bus_message *m = NULL;
        sd_bus_error *error = NULL;

        if(!is_bus_connected(lb_ctx)) {
                fprintf(stderr, "Bus is not opened\n");
                sd_bus_error_free(error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_call_method(lb_ctx->bus, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", "StartDiscovery", error, &m,
        NULL);
        if(r < 0) {
                fprintf(stderr, "Failed to issue method call: %s\n", error->message);
                sd_bus_error_free(error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sleep(seconds);

        r = sd_bus_call_method(lb_ctx->bus, "org.bluez", "/org/bluez/hci0", "org.bluez.Adapter1", "StopDiscovery", error, &m,
        NULL);

        if(r < 0) {
                fprintf(stderr, "Failed to issue method call: %s\n", error->message);
                sd_bus_error_free(error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sd_bus_error_free(error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
lb_open_system_bus(lb_context *lb_ctx)
{
        int r;
        /* Connect to the system bus */
        r = sd_bus_open_system(&lb_ctx->bus);
        if(r < 0) {
                fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
        }
        return r;
}

void
lb_close_system_bus(lb_context *lb_ctx)
{
        sd_bus_unref(lb_ctx->bus);
        if(lb_ctx->devices) {
                free(lb_ctx->devices);
                lb_ctx->devices_size = 0;
        }
}

int
lb_list_devices(lb_context *lb_ctx, int seconds)
{
        printf("Method Called: %s\n", __FUNCTION__);
        const char* *objects;
        const char *point;
        int i = 0, r = 0;
        sd_bus_error *error = NULL;

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
        //	fprintf(stderr, "Error getting root objects\n");
        //	free(objects);
        //	return -1;
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
        printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error *error = NULL;

        if(!is_bus_connected(lb_ctx)) {
                fprintf(stderr, "Bus is not opened\n");
                sd_bus_error_free(error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               "org.bluez",
                               address,
                               "org.bluez.Device1",
                               "Connect",
                               error,
                               &m,
                               NULL);

        if(r < 0) {
                fprintf(stderr, "Failed to issue method call: %s\n", error->message);
                sd_bus_error_free(error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sd_bus_error_free(error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
lb_disconnect_device(lb_context *lb_ctx, const char  *address)
{
        printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error *error = NULL;

        if(!is_bus_connected(lb_ctx)) {
                fprintf(stderr, "Bus is not opened\n");
                sd_bus_error_free(error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               "org.bluez",
                               address,
                               "org.bluez.Device1",
                               "Disconnect",
                               error,
                               &m,
                               NULL);

        if(r < 0) {
                sd_bus_error_free(error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sd_bus_error_free(error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
lb_pair_device(lb_context *lb_ctx, const char  *address)
{
        printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error *error = NULL;

        if(!is_bus_connected(lb_ctx)) {
                fprintf(stderr, "Bus is not opened\n");
                sd_bus_error_free(error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               "org.bluez",
                               address,
                               "org.bluez.Device1",
                               "Pair",
                               error,
                               &m,
                               NULL);

        if(r < 0) {
                fprintf(stderr, "Failed to issue method call: %s\n", error->message);
                sd_bus_error_free(error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        sd_bus_error_free(error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
lb_unpair_device(lb_context *lb_ctx, const char  *address)
{
        printf("Method Called: %s\n", __FUNCTION__);
        int r;
        sd_bus_message *m = NULL;
        sd_bus_error *error = NULL;

        if(!is_bus_connected(lb_ctx)) {
                fprintf(stderr, "Bus is not opened\n");
                sd_bus_error_free(error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        r = sd_bus_call_method(lb_ctx->bus,
                               "org.bluez",
                               address,
                               "org.bluez.Device1",
                               "CancelPairing",
                               error,
                               &m,
                               NULL);

        if(r < 0) {
                fprintf(stderr, "Failed to issue method call: %s\n", error->message);
                sd_bus_error_free(error);
                sd_bus_message_unref(m);
                return EXIT_FAILURE;
        }

        printf("Method Called: %s\n", "CancelPairing");

        sd_bus_error_free(error);
        sd_bus_message_unref(m);
        return EXIT_SUCCESS;
}

int
lb_write_to_char(lb_context *lb_ctx, const char *address, const char *value)
{
        printf("Method Called: %s\n", __FUNCTION__);
        return EXIT_SUCCESS;
}

int
main(int argc, char *argv[])
{
        int i = 0, r = 0;
        lb_context *lb_ctx = malloc(sizeof(lb_context));
        if (r < 0)
        {
                exit(r);
        }
        lb_ctx->bus = NULL;
        lb_ctx->devices = NULL;
        lb_ctx->devices_size = 0;

        lb_open_system_bus(lb_ctx);
        lb_list_devices(lb_ctx, 5);
        for(i = 0; i < lb_ctx->devices_size; i++) {
                printf("%s\t%s\n", lb_ctx->devices[i]->address, lb_ctx->devices[i]->name);
        }
        lb_connect_device(lb_ctx, "/org/bluez/hci0/dev_98_4F_EE_0F_42_B4");
        //lb_pair_device(lb_ctx, "/org/bluez/hci0/dev_98_4F_EE_0F_42_B4");
        //lb_unpair_device(lb_ctx, "/org/bluez/hci0/dev_98_4F_EE_0F_42_B4");
        lb_disconnect_device(lb_ctx, "/org/bluez/hci0/dev_98_4F_EE_0F_42_B4");
        lb_close_system_bus(lb_ctx);
        return 0;
}
