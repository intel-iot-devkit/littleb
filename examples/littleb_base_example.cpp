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
* @todo complete and document
*/
using namespace littleb;

int
main(int argc, char* argv[])
{
    try {
        DeviceManager devManager = DeviceManager::getInstance();
        devManager.getBlDevices();

        Device* firmata = devManager.getDeviceByName(std::string("FIRMATA"));

        BlProperties firmataProperties = firmata->getDeviceProperties();
        printf("Firmata state: connected: %d paired: %d truested: %d\n",
               firmataProperties.connected, firmataProperties.paired, firmataProperties.trusted);
        firmata->connect();
        printf("Firmata connected\n");
        firmata->disconnect();
        printf("Firmata disconnected\n");
    } catch (std::exception& e) {
        printf("%s\n", e.what());
    }
    return 0;
}