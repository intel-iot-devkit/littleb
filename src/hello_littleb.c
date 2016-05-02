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
#include <pthread.h>

int
main(int argc, char *argv[])
{
        int i = 0, j = 0, r = 0;
        ble_service **services = NULL;
        lb_context *lb_ctx = NULL;

        r = lb_context_new(&lb_ctx);
        if (r < 0) {
                fprintf(stderr, "ERROR: lb_context_new\n");
                exit(r);
        }

        r = lb_open_system_bus(&lb_ctx);
        if (r < 0) {
                fprintf(stderr, "ERROR: lb_open_system_bus\n");
                exit(r);
        }

        r = lb_get_bl_devices(lb_ctx, 5);
        if (r < 0) {
                fprintf(stderr, "ERROR: lb_get_bl_devices\n");
                exit(r);
        }
        for(i = 0; i < lb_ctx->devices_size; i++) {
                printf("%s\t%s\n", lb_ctx->devices[i]->address, lb_ctx->devices[i]->name);
        }

        // search for our specific device named "FIRMATA"
        bl_device* firmata = NULL;
        r = lb_get_device_by_device_name(lb_ctx, "FIRMATA", &firmata);
        if (r < 0) {
                fprintf(stderr, "ERROR: lb_get_device_by_device_name\n");
                exit(r);
        }

        r = lb_connect_device(lb_ctx, firmata);
        if (r < 0) {
                fprintf(stderr, "ERROR: lb_connect_device\n");
                exit(r);
        }

        r = lb_pair_device(lb_ctx, firmata);
        if (r < 0) {
                fprintf(stderr, "ERROR: lb_pair_device\n");
                exit(r);
        }

        r = lb_get_ble_device_services(lb_ctx, firmata, services);
        if (r < 0) {
                fprintf(stderr, "ERROR: lb_get_ble_device_services\n");
                exit(r);
        }

        printf("Device Found:\nName: %s\nDevice Address: %s\n", firmata->name, firmata->address);
        printf("Services found:\n");
        for(i = 0; i < firmata->services_size; i++) {
                printf("%s\t%s\n", firmata->services[i]->service_path, firmata->services[i]->uuid);
                printf("Characteristics Found:\n");
                for (j = 0; j < firmata->services[i]->characteristics_size; j++) {
                        printf("%s\t%s\n", firmata->services[i]->characteristics[j]->char_path, firmata->services[i]->characteristics[j]->uuid);
                }
        }

        printf("Blinking...\n");
        uint8_t led_on[] = { 0x91, 0x20, 0x00 };
        uint8_t led_off[] = { 0x91, 0x00, 0x00 };
        for(i = 0; i < 20; i++) {
                r = lb_write_to_characteristic(lb_ctx, firmata, "6e400002-b5a3-f393-e0a9-e50e24dcca9e", 3, led_on);
                if (r < 0) {
                        fprintf(stderr, "ERROR: lb_write_to_characteristic\n");
                }
                sleep(0.5);
                r = lb_write_to_characteristic(lb_ctx, firmata, "6e400002-b5a3-f393-e0a9-e50e24dcca9e", 3, led_off);
                if (r < 0) {
                        fprintf(stderr, "ERROR: lb_write_to_characteristic\n");
                }
                sleep(0.5);
        }

        //r = lb_unpair_device(lb_ctx, firmata);
        //if (r < 0) {
        //        fprintf(stderr, "ERROR: lb_unpair_device\n");
        //        exit(r);
        //}

        r = lb_disconnect_device(lb_ctx, firmata);
        if (r < 0) {
                fprintf(stderr, "ERROR: lb_disconnect_device\n");
                exit(r);
        }

        r = lb_close_system_bus(lb_ctx);
        if (r < 0) {
                fprintf(stderr, "ERROR: lb_close_system_bus\n");
                exit(r);
        }

        printf("Done\n");

        return 0;
}
