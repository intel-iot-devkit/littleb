/*
 * Author: Ofra Moyal Cohen <ofra.moyal.cohen@intel.com>
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
#include <stdio.h>
#include <stdlib.h>

static int
test1_callback(lb_bl_property_change_notification input, void* userdata)
{
    printf("input : %d\n", input);
    const char* userdata_test = (const char*) userdata;
    printf("callback called with userdata: %s\n", userdata_test);
    return 0;
}

int
main(int argc, char* argv[])
{
    int i = 0, j = 0, r = 0;

    r = lb_init();
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_init\n");
        exit(r);
    }

    r = lb_get_bl_devices(5);
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_get_bl_devices\n");
        goto cleanup;
    }

    // search for our specific device named "FIRMATA"
    lb_bl_device* firmata = NULL;
    r = lb_get_device_by_device_name("FIRMATA", &firmata);
    if (r < 0) {
        fprintf(stderr, "ERROR: Device FIRMATA not found\n");
        goto cleanup;
    }

    r = lb_register_change_state_event(firmata, test1_callback, NULL);
    if (r < 0) {
        printf("failed to register change event\n");
    }

    r = lb_connect_device(firmata);
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_connect_device\n");
        goto cleanup;
    }

    r = lb_disconnect_device(firmata);
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_disconnect_device\n");
        goto cleanup;
    }
/*
r = lb_unpair_device(firmata);
if (r < 0) {
    fprintf(stderr, "ERROR: lb_unpair_device\n");
    goto cleanup;
}

r = lb_pair_device(firmata);
if (r < 0) {
    fprintf(stderr, "ERROR: lb_pair_device\n");
    goto cleanup;
}
*/


cleanup:

    r = lb_disconnect_device(firmata);
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
