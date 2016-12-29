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
#include "../api/littleb.hpp"

/**
* @todo complete and document, replace printf with cout
*/
using namespace littleb;
using namespace std;

static int
test_callback(sd_bus_message* message, void* userdata, sd_bus_error* error)
{
    uint8_t* result = NULL;
    const char* userdata_test = (const char*) userdata;

    printf("callback called with userdata: %s\n", userdata_test);

    try {
        result = Device::parseUartServiceMessage(message);
    } catch (exception& e) {
        fprintf(stderr, "ERROR: couldn't parse uart message, %s\n", e.what());
        return -1;
    }

    printf("message is:\n");
    int size = sizeof(result) / sizeof(uint8_t);

    for (int i = 0; i < size; i++) {
        printf("%x ", result[i]);
    }
    printf("\n");

    return 0;
}


int
main(int argc, char* argv[])
{
    try {
        DeviceManager& devManager = DeviceManager::getInstance();
        devManager.getBlDevices();

        Device* firmata = devManager.getDeviceByName(string("FIRMATA"));

        firmata->connect();

        firmata->getBleDeviceServices();

        printf("Device Found:\nName: %s\nDevice Address: %s\n", firmata->getName().c_str(),
               firmata->getAddress().c_str());

        BleService* services = firmata->getServices();

        if (services == NULL) {
            printf("No services found:\n");
        } else {
            printf("Services found:\n");

            for (int i = 0; i < firmata->getNumOfServices(); i++) {
                printf("%s\t%s\n", services[i].getPath().c_str(), services[i].getUuid().c_str());
                printf("Characteristics Found:\n");

                const vector<BleCharactersitic*> characteristics = services[i].getCharacteristics();
                for (unsigned int j = 0; j < characteristics.size(); j++) {
                    printf("%s\t%s\n", characteristics[j]->getPath().c_str(),
                           characteristics[j]->getUuid().c_str());
                }
            }

            delete[] services;
        }

        printf("Blinking");
        fflush(stdout);
        uint8_t led_on[] = { 0x91, 0x20, 0x00 };
        uint8_t led_off[] = { 0x91, 0x00, 0x00 };
        for (int i = 0; i < 10; i++) {
            printf(".");
            fflush(stdout);
            firmata->writeToCharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e", 3, led_on);

            usleep(1000000);
            printf(".");
            fflush(stdout);
            firmata->writeToCharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e", 3, led_off);

            usleep(1000000);
        }
        printf("\n");

        const char* userdata_test = "test";
        firmata->registerCharacteristicReadEvent("6e400003-b5a3-f393-e0a9-e50e24dcca9e",
                                                 test_callback, (void*) userdata_test);

        printf("get_version\n");
        fflush(stdout);
        uint8_t get_version[] = { 0xf0, 0x79, 0xf7 };

        firmata->writeToCharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e", 3, get_version);

        printf("waiting for callbacks\n");
        fflush(stdout);
        sleep(2);

        // read device properties
        BlProperties firmataProperties = firmata->getDeviceProperties();
        printf("Firmata state: connected: %d paired: %d truested: %d\n",
               firmataProperties.connected, firmataProperties.paired, firmataProperties.trusted);

        firmata->disconnect();
        printf("Firmata disconnected\n");

    } catch (exception& e) {
        printf("%s\n", e.what());
    }
    return 0;
}
