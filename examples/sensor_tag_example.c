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

#include "littleb.h"
#include <stdio.h>

static const char* IR_TEMP_DATA_UUID = "f000aa01-0451-4000-b000-000000000000";
static const char* IR_TEMP_CONFIG_UUID = "f000aa02-0451-4000-b000-000000000000";

static int
temp_sensor_callback(sd_bus_message* message, void* userdata, sd_bus_error* error)
{
    const char* userdata_test = (const char*) userdata;
    printf("Callback with uuid: %s\n", userdata_test);

    int i, r;
    size_t size = 0;
    uint8_t* result = NULL;

    r = lb_parse_dbus_message(message, (const void**) &result, &size);

    if (r < 0 || size < 4) {
        fprintf(stderr, "ERROR: couldn't parse message\n");
        return -1;
    }

    printf("raw data recived: ");
    for (i = 0; i < size; i++) {
        printf("%d ", result[i]);
    }
    printf("\n");

    // basic tempreture calculation, see TI examples for more accurate results
    uint16_t rawTemp = (result[1] << 8) | result[0];
    printf("IR temperature: %.1f\n", ((double) rawTemp) / 128.0);

    uint16_t rawAmbTemp = (result[3] << 8) | result[2];
    printf("ambient temperature: %.1f\n", ((double) rawAmbTemp) / 128.0);

    return 0;
}

int
main(int argc, char* argv[])
{
    int i = 0, j = 0, r = 0;
    size_t size = 0;
    uint8_t* result = NULL;

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

    // search for our specific device
    lb_bl_device* sensorTag = NULL;
    r = lb_get_device_by_device_name("CC2650 SensorTag", &sensorTag);
    if (r < 0) {
        fprintf(stderr, "ERROR: Sensor Tag 2.0 not found\n");
        goto cleanup;
    }

    r = lb_connect_device(sensorTag);
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_connect_device\n");
        goto cleanup;
    }

    r = lb_get_ble_device_services(sensorTag);
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_get_ble_device_services\n");
        goto cleanup;
    }

    printf("Device Found:\nName: %s\nDevice Address: %s\n\n", sensorTag->name, sensorTag->address);
    printf("Services and characteristics:\n");
    printf("Services found:\n");
    for (i = 0; i < sensorTag->services_size; i++) {
        printf("%s\t%s\n", sensorTag->services[i]->service_path, sensorTag->services[i]->uuid);
        printf("Characteristics Found:\n");
        for (j = 0; j < sensorTag->services[i]->characteristics_size; j++) {
            printf("%s\t%s\n", sensorTag->services[i]->characteristics[j]->char_path,
                   sensorTag->services[i]->characteristics[j]->uuid);
        }
    }


    uint8_t flagOn = 0x01;

    printf("\nEnable IR Temperature Sensor\n");
    r = lb_write_to_characteristic(sensorTag, IR_TEMP_CONFIG_UUID, 1, &flagOn);
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_write_to_characteristic\n");
    }

    // enable sensor notifications
    const char* userdata = "temperature sensor test";
    r = lb_register_characteristic_read_event(sensorTag, IR_TEMP_DATA_UUID, temp_sensor_callback,
                                              (void*) userdata);
    if (r < 0) {
        fprintf(stderr, "ERROR: lb_register_characteristic_read_event\n");
    }

    printf("waiting for callbacks\n");
    fflush(stdout);
    sleep(2);

cleanup:
    r = lb_disconnect_device(sensorTag);
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
