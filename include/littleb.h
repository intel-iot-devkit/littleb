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
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <syslog.h>
#include <systemd/sd-bus.h>

#define MAX_LEN 256
#define MAX_OBJECTS 256

#define DEBUG 0

static const char *BLUEZ_DEST = "org.bluez";
static const char *BLUEZ_DEVICE = "org.bluez.Device1";
static const char *BLUEZ_GATT_SERVICE = "org.bluez.GattService1";
static const char *BLUEZ_GATT_CHARACTERISTICS = "org.bluez.GattCharacteristic1";

/**
 * LB return codes
 */
typedef enum {
    LB_SUCCESS = 0,                             /**< Expected response */
    LB_ERROR_FEATURE_NOT_IMPLEMENTED = 1,       /**< Feature TODO */
    LB_ERROR_FEATURE_NOT_SUPPORTED = 2,         /**< Feature not supported by HW */
    LB_ERROR_INVALID_VERBOSITY_LEVEL = 3,       /**< Verbosity level wrong */
    LB_ERROR_INVALID_PARAMETER = 4,             /**< Parameter invalid */
    LB_ERROR_INVALID_HANDLE = 5,                /**< Handle invalid */
    LB_ERROR_NO_RESOURCES = 6,                  /**< No resource of that type avail */

    LB_ERROR_UNSPECIFIED = 99 /**< Unknown Error */
} lb_result_t;

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
        ble_char **characteristics;
        int characteristics_size;
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

int lb_init();
int lb_destroy();
int lb_context_new(lb_context **lb_ctx);
int lb_context_free(lb_context **lb_ctx);
int lb_get_bl_devices(lb_context *lb_ctx, int seconds);
int lb_connect_device(lb_context *lb_ctx, bl_device* bl_dev);
int lb_disconnect_device(lb_context *lb_ctx, bl_device* bl_dev);
int lb_pair_device(lb_context *lb_ctx, bl_device* bl_dev);
int lb_unpair_device(lb_context *lb_ctx, bl_device* bl_dev);
int lb_get_ble_characteristic_by_characteristic_path(lb_context *lb_ctx, bl_device *bl_dev, const char *characteristic_path, ble_char **ble_characteristic_ret);
int lb_get_ble_characteristic_by_uuid(lb_context *lb_ctx, bl_device *bl_dev, const char *uuid, ble_char **ble_characteristic_ret);
int lb_get_ble_service_by_service_path(lb_context *lb_ctx, bl_device *bl_dev, const char *service_path, ble_service **ble_service_ret);
int lb_get_ble_service_by_uuid(lb_context *lb_ctx, bl_device *bl_dev, const char *uuid, ble_service **ble_service_ret);
int lb_get_ble_device_services(lb_context *lb_ctx, bl_device* bl_dev, ble_service **services);
int lb_get_device_by_device_path(lb_context *lb_ctx, const char *device_path, bl_device **bl_device_ret);
int lb_get_device_by_device_name(lb_context *lb_ctx, const char *name, bl_device **bl_device_ret);
int lb_get_device_by_device_address(lb_context *lb_ctx, const char *address, bl_device **bl_device_ret);
int lb_write_to_characteristic(lb_context *lb_ctx, bl_device *bl_dev, const char* uuid, int size, uint8_t *value);
int lb_read_from_characteristic(lb_context *lb_ctx, bl_device *bl_dev, const char* uuid, size_t *size, uint8_t **result);
int lb_register_for_device_data(lb_context *lb_ctx, sd_bus_message_handler_t callback, void *userdata);

#ifdef __cplusplus
}
#endif
