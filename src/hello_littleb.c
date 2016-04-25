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

int
main(int argc, char *argv[])
{
        int i = 0, r = 0;
        ble_service **services = NULL;
        lb_context *lb_ctx = malloc(sizeof(lb_context));
        if (r < 0)
        {
                exit(r);
        }
        lb_ctx->bus = NULL;
        lb_ctx->devices = NULL;
        lb_ctx->devices_size = 0;

        lb_open_system_bus(lb_ctx);

        lb_get_bl_devices(lb_ctx, 5);
        for(i = 0; i < lb_ctx->devices_size; i++) {
                printf("%s\t%s\n", lb_ctx->devices[i]->address, lb_ctx->devices[i]->name);
        }

        bl_device* firmata = NULL;
        r = lb_get_device_by_device_name(lb_ctx, "FIRMATA", &firmata);
        if (r < 0) {
                fprintf(stderr, "ERROR\n");
                exit(r);
        }

        lb_connect_device(lb_ctx, firmata->device_path);

        lb_pair_device(lb_ctx, firmata->device_path);

        lb_get_ble_device_services(lb_ctx, firmata->device_path, services);

        sleep(2);

        lb_unpair_device(lb_ctx, firmata->device_path);

        lb_disconnect_device(lb_ctx, firmata->device_path);

        lb_close_system_bus(lb_ctx);

        return 0;
}
