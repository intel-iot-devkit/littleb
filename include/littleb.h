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

#define DEBUG 1

typedef struct ble_characteristic
{
        const char *char_path;
        const char *uuid;
} ble_char;

typedef struct ble_service
{
        const char *service_path;
        const char *uuid;
        bool primary;
        ble_char **chars;
        int chars_size;
} ble_service;

typedef struct bl_device
{
        const char *device_path;
        const char *address;
        const char *name;
        ble_service **services;
        int services_size;
} bl_device;


typedef struct lb_context
{
        sd_bus *bus;
        bl_device **devices;
        int devices_size;
}lb_context;

int lb_open_system_bus(lb_context *lb_ctx);
int lb_close_system_bus(lb_context *lb_ctx);
int lb_get_bl_devices(lb_context *lb_ctx, int seconds);
int lb_connect_device(lb_context *lb_ctx, const char  *address);
int lb_disconnect_device(lb_context *lb_ctx, const char  *address);
int lb_pair_device(lb_context *lb_ctx, const char  *address);
int lb_unpair_device(lb_context *lb_ctx, const char  *address);
int lb_get_ble_device_services(lb_context *lb_ctx, const char* device_path, ble_service **services);
int lb_get_device_by_device_path(lb_context *lb_ctx, const char *device_path, bl_device **bl_device_pointer);
int lb_get_device_by_device_name(lb_context *lb_ctx, const char *name, bl_device **bl_device_pointer);
int lb_get_device_by_device_address(lb_context *lb_ctx, const char *address, bl_device **bl_device_pointer);

