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

#include "littleb.h"

#define MAX_LEN 256
#define MAX_OBJECTS 256
#define MSG_MAX_PROPERTIES \
    10 // max number of entries supported when parsing sd_bus_message, if there are more they won't
       // be handled for notifications

static const char* BLUEZ_DEST = "org.bluez";
static const char* BLUEZ_DEVICE = "org.bluez.Device1";
static const char* BLUEZ_GATT_SERVICE = "org.bluez.GattService1";
static const char* BLUEZ_GATT_CHARACTERISTICS = "org.bluez.GattCharacteristic1";

static const char* PAIRED_PROPERTY_NAME = "Paired";
static const char* TRUSTED_PROPERTY_NAME = "Trusted";
static const char* CONNECTED_PROPERTY_NAME = "Connected";

struct bl_context {
    sd_bus* bus;            /**< system bus to be used */
    lb_bl_device** devices; /**< list of the devices found in a scan */
    int devices_size;       /**< count of devices found*/
};

typedef struct bl_context* lb_context;

typedef struct event_matches_callbacks {
    // const char* event;
    char* event;
    sd_bus_message_handler_t callback;
    void* userdata;
} event_matches_callbacks;

typedef struct {
    property_change_callback_func callback;
    void* data;
} bl_user_notification_callback;

typedef struct {
    char* name;
    bool value;
} bl_property_data;


#ifdef __cplusplus
}
#endif
