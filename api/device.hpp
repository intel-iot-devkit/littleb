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

#pragma once

#include "littlebtypes.hpp"
#include <sstream>
#include <stdexcept>
#include <vector>

namespace littleb
{
/**
 * API to device
 *
 */
class Device
{
  public:
    /**
     * Constructor
     *
     * DeviceManager uses it to return device
     * Cannot call an empty constructor
     *
     * @throws std::invalid_argument in case of null input
     */
    Device(lb_bl_device* device)
    {
        if (device == NULL) {
            throw std::invalid_argument("Error initialising DeviceManager");
        } else {
            //@todo copy fields separately
            memcpy(&m_device, device, sizeof(lb_bl_device));
        }
    }

    /**
     * Destructor
     */
    ~Device()
    {
    }

    /**
     * Connect device
     *
     * @throws std::runtime_error when lb_connect_device() exit with an error
    */
    void
    connect()
    {
        if (lb_connect_device(&m_device) != LB_SUCCESS) {
            throw std::runtime_error("littleb device connect call failed");
        }
    }

    /**
     * Disconnect device
     *
     * @throws std::runtime_error when lb_disconnect_device() exit with an error
     */
    void
    disconnect()
    {
        if (lb_disconnect_device(&m_device) != LB_SUCCESS) {
            throw std::runtime_error("littleb device disconnect call failed");
        }
    }

    /**
     * Pair device
     *
     * @throws std::runtime_error when lb_pair_device() exit with an error
     */
    void
    pair()
    {
        if (lb_pair_device(&m_device) != LB_SUCCESS) {
            throw std::runtime_error("littleb device pair call failed");
        }
    }

    /**
     * Cancel pairing
     *
     * @throws std::runtime_error when lb_unpair_device() exit with an error
     */
    void
    unpair()
    {
        if (lb_unpair_device(&m_device) != LB_SUCCESS) {
            throw std::runtime_error("littleb device unpair call failed");
        }
    }

    /**
     * Populate BleCharactersitic with characteristic found by using it's device path under dbus
     *
     * @param characteristic_path to search for
     * @param BleCharactersitic to populate with characteristic found
     * @throws std::runtime_error when get characteristic call exit with an error
     */
    void
    getBleCharacteristicByCharacteristicPath(std::string path, BleCharactersitic* out)
    {
        lb_ble_char* bleCharNew = NULL;
        lb_result_t res = lb_get_ble_characteristic_by_characteristic_path(&m_device, path.c_str(), &bleCharNew);

        if (res != LB_SUCCESS) {
            std::ostringstream oss;
            oss << "littleb getBleCharacteristicByCharacteristicPath for path " << path
                << " failed";
            throw std::runtime_error(oss.str());
        }
        out = new BleCharactersitic(*bleCharNew);
    }

    /**
     * Populate BleCharactersitic with characteristic found by using it's uuid
     *
     * @param uuid to search for
     *  to populate with characteristic found
     * @throws std::runtime_error when get characteristic call exit with an error
     */
    void
    getBleCharacteristicByUuid(std::string uuid, BleCharactersitic* charecteristic)
    {
        lb_ble_char* bleCharNew = NULL;
        lb_result_t res = lb_get_ble_characteristic_by_uuid(&m_device, uuid.c_str(), &bleCharNew);

        if (bleCharNew == NULL || res != LB_SUCCESS) {
            std::ostringstream oss;
            oss << "littleb getBleCharacteristicByUuid for uuid " << uuid << " failed";
            throw std::runtime_error(oss.str());
        }
        charecteristic = new BleCharactersitic(*bleCharNew);
    }

    /**
     * Populate BleService with service found by using it's device path under dbus
     *
     * @param service path to search for
     * @param BleService to populate with service found
     * @throws std::runtime_error when get service call exit with an error or returned a null
     * service
     */
    void
    getBleServiceByServicePath(std::string path, BleService* out)
    {
        lb_ble_service* bleServiceNew = NULL;
        lb_result_t res = lb_get_ble_service_by_service_path(&m_device, path.c_str(), &bleServiceNew);

        if (bleServiceNew == NULL || res != LB_SUCCESS) {
            std::ostringstream oss;
            oss << "littleb getBleServiceByServicePath for path " << path << " failed";
            throw std::runtime_error(oss.str());
        }

        out = new BleService();
        out->init(*bleServiceNew);
    }

    /**
     * Populate BleService with service found by using it's uuid
     *
     * @param uuid to search for
     * @param BleService to populate with service found
     * @throws std::runtime_error when get service call exit with an error or returned a null
     * service
     */
    void
    getBleServiceByUuid(std::string uuid, BleService* out)
    {
        lb_ble_service* bleServiceNew = NULL;
        lb_result_t res = lb_get_ble_service_by_uuid(&m_device, uuid.c_str(), &bleServiceNew);

        if (bleServiceNew == NULL || res != LB_SUCCESS) {
            std::ostringstream oss;
            oss << "littleb getBleServiceByUuid for uuid " << uuid << " failed";
            throw std::runtime_error(oss.str());
        }

        out = new BleService();
        out->init(*bleServiceNew);
    }

    /**
     * Populate the BLE device with it's services
     * @throws std::runtime_error when get device services call exit with an error
     *
     */
    void
    getBleDeviceServices()
    {
        if (lb_get_ble_device_services(&m_device) != LB_SUCCESS) {
            throw std::runtime_error("littleb device getBleDeviceServices call failed");
        }
    }

    /**
     * Write to a specific BLE device characteristic using it's uuid
     *
     * @param uuid of the characteristic to write to
     * @param size of the uint8 array to be written
     * @param the array of byte buffer to write to the characteristic
     * @throws std::runtime_error when lb_write_to_characteristic() call exit with an error
     */
    void
    writeToCharacteristic(std::string uuid, int size, uint8_t* value)
    {
        if (lb_write_to_characteristic(&m_device, uuid.c_str(), size, value)) {
            throw std::runtime_error("littleb device writeToCharacteristic call failed");
        }
    }

    /**
     * Read from a specific BLE device characteristic using it's uuid
     *
     * @param uuid of the characteristic to read from
     * @param size of the uint8 array that was read
     * @param the array of byte buffer that was read
     * @throws std::runtime_error when lb_read_from_characteristic() call exit with an error
     */
    void
    readFromCharacteristic(std::string uuid, size_t* size, uint8_t** result)
    {
        if (lb_read_from_characteristic(&m_device, uuid.c_str(), size, result)) {
            throw std::runtime_error("littleb device readFromCharacteristic call failed");
        }
    }

    /**
     * Get device properties
     *
     * @throws std::runtime_error when lb_get_device_properties() call exit with an error
     * @return device properties
     */
    BlProperties
    getDeviceProperties()
    {
        lb_bl_properties tempProps;

        if (lb_get_device_properties(m_device.address, &tempProps) != LB_SUCCESS) {
            throw std::runtime_error("littleb device getDeviceProperties call failed");
        }

        BlProperties outProps;
        outProps.trusted = tempProps.trusted;
        outProps.paired = tempProps.paired;
        outProps.connected = tempProps.connected;
        return outProps;
    }

  private:
    lb_bl_device m_device;
    Device();
};
}