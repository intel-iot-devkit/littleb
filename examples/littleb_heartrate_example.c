/*
 * Author: Ofra Moyal Cohen <ofra.moyal.cohen@intel.com>
 * Copyright (c) 2017 Intel Corporation.
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
#include <stdio.h>

/*
    This sample shows how to connect with Arduino device running Zephyr
    In order to run this successfully please load an Arduino device with peripheral_hr Zephyr sample
*/
static const int MAX_CONNECT_ATTEMPTS = 5;
static const char* MESURMENT_UUID = "00002a37-0000-1000-8000-00805f9b34fb";
static const char* DEVICE_NAME = "Zephyr Heartrate Sensor";

static int
heartrate_callback(sd_bus_message* message, void* userdata, sd_bus_error* error)
{
    const char* userdata_test = (const char*) userdata;

    int r;
    size_t size = 0;
    uint8_t* result = NULL;

    r = lb_parse_dbus_message(message, (const void**) &result, &size);
    if (r < 0) {
        fprintf(stderr, "ERROR: couldn't parse uart message\n");
        return -1;
    }

    // the heartrate is stored in the second place in results array
    if (size >= 2) {
        printf("Heartrate %d bpm\n", result[1]);
    }

    return 0;
}

int

main(int argc, char* argv[])
{
    int i = 0, r = 0;

    r = lb_init();
    if (r < 0) {
        fprintf(stderr, "ERROR: failed to init littleb\n");
        exit(r);
    }

    r = lb_get_bl_devices(5);
    if (r < 0) {
        fprintf(stderr, "ERROR: failed to get list of available devices\n");
        goto cleanup;
    }

    lb_bl_device* zepher = NULL;

    r = lb_get_device_by_device_name(DEVICE_NAME, &zepher);
    if (r < 0) {
        fprintf(stderr, "ERROR: %s was not found\n", DEVICE_NAME);
        goto cleanup;
    }

    // allow few attempts when trying to connect to device
    printf("Conncting to %s..", DEVICE_NAME);
    fflush(stdout);

    do {
        r = lb_connect_device(zepher);
        printf(".");
        fflush(stdout);
        i++;
    } while (r < 0 && i < MAX_CONNECT_ATTEMPTS);

    printf("\n");
    fflush(stdout);

    if (r < 0) {
        fprintf(stderr, "ERROR: failed to connect to %s\n", DEVICE_NAME);
        goto cleanup;
    }

    printf("Connected successfully\n");

    r = lb_get_ble_device_services(zepher);
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_get_ble_device_services\n");
        goto cleanup;
    }

    const char* userdata = "";
    r = lb_register_characteristic_read_event(zepher, MESURMENT_UUID, heartrate_callback, (void*) userdata);
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_register_characteristic_read_event %s\n", MESURMENT_UUID);
        goto cleanup;
    }

    fflush(stdout);
    sleep(2);

cleanup:
    r = lb_disconnect_device(zepher);
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_disconnect_device\n");
    }

    r = lb_destroy();
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_destroy\n");
    }

    printf("Done\n");

    return 0;
}
