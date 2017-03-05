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
#include "../api/device.h"
#include "../api/devicemanager.h"
#include <iostream>
int printResult(std::vector<uint8_t>& res, sd_bus_message* message, void* userdata)
{
    try {
        res = parseUartServiceMessage(message);
    } catch (std::exception& e) {
        std::cout <<"ERROR: couldn't parse uart message, "<<e.what() <<std::endl;
        return -1;
    }

    std::cout<< "raw data recived: "<<std::endl;
    
    for (unsigned int i = 0; i < res.size(); i++) {
        int r = (int) res[i];
        std::cout << r<<" ";
    }
    std::cout << std::endl;
    return 0;
}

int sensor_callback(sd_bus_message* message, void* userdata, sd_bus_error* error)
{
    
    std::vector<uint8_t> res;
    if (printResult(res, message, userdata) != 0) {
        return -1;
    }
   
    uint16_t rawTemp = (res[1] << 8) | res[0];
    std::cout <<"\nIR temperature: "<<((double) rawTemp) / 128.0 <<std::endl;

    uint16_t rawAmbTemp = (res[3] << 8) | res[2];
    std::cout <<"ambient temperature: "<<((double) rawAmbTemp) / 128.0 <<std::endl;

    return 0;
}

int firmata_callback(sd_bus_message* message, void* userdata, sd_bus_error* error)
{
    
    std::vector<uint8_t> result;
    if (printResult(result, message, userdata) != 0) {
        return -1;
    }
    return 0;
}
int change_state_callback(lb_bl_property_change_notification bcn, void* userdata) 
{
    const char* userdata_test = (const char*) userdata;
    std::cout<<"change_state_callback with userdata: "<< userdata_test<<std::endl;
    std::cout<<"Event changed: "<< bcn<<std::endl;
    

    return 0;
}
#define DEVICE "FIRMATA"
std::shared_ptr<Device> firmata;
int
main(int argc, char* argv[])
{
    try {
        DeviceManager& devManager = DeviceManager::getInstance();
        devManager.getBlDevices();

	firmata = devManager.getDeviceByName(DEVICE);

        firmata->connect();

        std::vector<std::shared_ptr<BleService>> services = firmata->getBleDeviceServices();

        std::cout<<"Device Found:\nName: "<<firmata->getName()<<"\nDevice Address: "<<firmata->getAddress()<<std::endl;

        if (services.size() == 0) {
            std::cout<<"No services found"<<std::endl;
        } else {
            std::cout<<"Services found:"<<std::endl;

            for (int i = 0; i < firmata->getNumOfServices(); i++) {
                std::cout<<services[i]->getPath() << "\t" << services[i]->getUuid()<<std::endl;
                std::cout<<"Characteristics Found:"<<std::endl;

                std::vector<std::shared_ptr<BleCharactersitic>> characteristics = services[i]->getCharacteristics();
                for (unsigned int j = 0; j < characteristics.size(); j++) {
                    std::cout<< characteristics[j]->getPath()<< "\t" << 
                           characteristics[j]->getUuid()<<std::endl;
                }
            }

            // delete[] services;
        }

        std::cout <<"Blinking";
        std::vector<uint8_t> led_on = { 0x91, 0x20, 0x00 };
        std::vector<uint8_t> led_off = { 0x91, 0x00, 0x00 };
        for (int i = 0; i < 10; i++) {
            std::cout <<".";
            fflush(stdout);
            firmata->writeToCharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e", 3, led_on);

            usleep(100000);
            std::cout <<".";
            fflush(stdout);
            firmata->writeToCharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e", 3, led_off);

            usleep(100000);
        }

        char* userdata_test = "test";
        
        
        firmata->registerChangeStateEvent(change_state_callback, userdata_test);
        firmata->registerCharacteristicReadEvent("6e400003-b5a3-f393-e0a9-e50e24dcca9e", firmata_callback, userdata_test);
        
        std::cout <<std::endl<<"get_version"<<std::endl;
        // fflush(stdout);
        std::vector<uint8_t> get_version = { 0xf0, 0x79, 0xf7 };

        firmata->writeToCharacteristic("6e400002-b5a3-f393-e0a9-e50e24dcca9e", 3, get_version);

        std::cout <<"waiting for callbacks"<<std::endl;
        // fflush(stdout);
        sleep(2);

        // read device properties
        BlProperties firmataProperties = firmata->getDeviceProperties();
        std::cout <<"Firmata state: connected: "<< firmataProperties.connected <<" paired: "<<
                    firmataProperties.paired <<" truested: "<<firmataProperties.trusted <<std::endl;

        firmata->disconnect();
        std::cout <<"Firmata disconnected"<<std::endl;
        sleep(4);
    } catch (std::exception& e) {
        std::cout << e.what()<<std::endl;
	firmata->disconnect();
        std::cout <<"Firmata disconnected"<<std::endl;
    }
    return 0;
}
