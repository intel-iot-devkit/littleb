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
#include "littleb.h"

#define BT_UUID_ESS         "0000181a-0000-1000-8000-00805f9b34fb"
#define BT_UUID_CUD         "00002901-0000-1000-8000-00805f9b34fb"
#define BT_UUID_TEMPERATURE "00002a6e-0000-1000-8000-00805f9b34fb"
#define BT_UUID_MEASUREMENT "0000290c-0000-1000-8000-00805f9b34fb"
#define BT_NOFITY_FLAG      "notify"


static int
test_callback(sd_bus_message* message, void* userdata, sd_bus_error* error)
{

    int r, i;
    size_t size = 0;
    uint8_t* result = NULL;
    const char* userdata_test = (const char*) userdata;

    printf("callback called with userdata: %s\n", userdata_test);

    r = lb_parse_uart_service_message(message, (const void**) &result, &size);
    if (r < 0) {
        fprintf(stderr, "ERROR: couldn't parse uart message\n");
        return -1;
    }

    printf("message is:\n");
    for (i = 0; i < size; i++) {
        printf("%x ", result[i]);
    }
    printf("\n");

    return 0;
}

int
main(int argc, char* argv[])
{
    int r = 0;
    unsigned int i, num_devices;
    lb_bl_device* device = NULL;

    r = lb_init();
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_init\n");
        exit(r);
    }

    lb_context lb_ctx = lb_context_new();
    if (lb_ctx == NULL) {
        fprintf(stderr, "ERROR: lb_context_new\n");
        exit(r);
    }

    // Discovery
    r = lb_start_device_discovery(lb_ctx);
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_get_bl_devices\n");
        goto cleanup;
    }
    printf("Started device discovery\n");
    sleep(5);
    r = lb_get_bl_devices_no_scan(lb_ctx);
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_get_bl_devices_no_scan\n");
        goto cleanup;
    }
    r = lb_stop_device_discovery(lb_ctx);

    r = lb_get_bl_device_count(lb_ctx, &num_devices);
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_get_bl_device_count\n");
        goto cleanup;
    }
    printf("Found %d devices\n", num_devices);

    // Look for ESP device
    bool found_esp_device = false;
    lb_bl_device* dev;
    lb_ble_service* service;
    for (i = 0; i < num_devices && !found_esp_device; i++) {
        r = lb_get_bl_device_by_index(lb_ctx, i, &dev);
        if (r == LB_SUCCESS) {
            if (strcmp(dev->name, "null") && dev->rssi > 0) {
                printf("Checking \"%s\" %s...", dev->name, dev->address);
                fflush(stdout);
                printf("\n");

                r = lb_connect_device(lb_ctx, dev);
                if (r == LB_SUCCESS) {
                    fprintf(stderr, "Connected\n");
                    r = lb_get_ble_device_services(lb_ctx, dev);
                    // r = lb_get_ble_service_by_uuid(lb_ctx, dev, BT_UUID_ESS, &service);
                    if (r == LB_SUCCESS) {
                        fprintf(stderr, "Found %d services\n", dev->services_size);
                        for (int s = 0; s < dev->services_size; s++) {
                            printf("%s\t%s\n", dev->services[s]->service_path, dev->services[s]->uuid);
                        }
                        // r = lb_disconnect_device(lb_ctx, dev);
                    } else {
                        fprintf(stderr, "Could not get services\n");
                    }
                }

                /*
                r = lb_connect_device(lb_ctx, dev);
                if (r == LB_SUCCESS) {
                    fprintf(stderr, "Success\n");
                    r = lb_disconnect_device(lb_ctx, dev);
                } else {
                    fprintf(stderr, "Failed\n");
                }
                */
            }
        } else {
            fprintf(stderr, "ERROR: lb_get_bl_device_by_index failed for index %d\n", i);
        }
    }

cleanup:
    r = lb_context_free(lb_ctx);
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_context_free\n");
    }

    r = lb_destroy();
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_destroy\n");
    }

    printf("Done\n");

    return 0;
}
