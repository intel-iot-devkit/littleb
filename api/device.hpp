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
#include <stdexcept>
#include <sstream>
#include <vector>

namespace littleb
{

	//@todo review and fix all documentation

class Device
{
	public:
		Device(bl_device* device)
		{
			memcpy(&m_device, device, sizeof(device));
		}

		~Device();

		/**
		 * Connect to device
		 *
		 * @return Result of operation
		 */
		Result connect()
		{
			return (Result)lb_connect_device(&m_device);
		}

		/**
		 * Disconnect device
		 *
		 * @return Result of operation
		 */
		Result disconnect()
		{
			return (Result)lb_disconnect_device(&m_device);
		}

		/**
		 * Pair device
		 *
		 * @return Result of operation
		 */
		Result pair()
		{
			return (Result)lb_pair_device(&m_device);
		}

		/**
		 * Cancel pairing 
		 *
		 * @return Result of operation
		 */
		Result unpair()
		{
			return (Result)lb_unpair_device(&m_device);
		}

		/**
		 * Populate ble_char with characteristic found by using it's device path under dbus
		 *
		 * @param characteristic_path to search for
		 * @param ble_char to populate with characteristic found
		 * @return Result of operation
		 */
		void getBleCharacteristicByCharacteristicPath(std::string path, BleCharactersitic* charecteristic)
		{
			lb_ble_char* bleCharNew = NULL;
			lb_result_t res = lb_get_ble_characteristic_by_characteristic_path(&m_device, path.c_str(), &bleCharNew);
			
			if (res != LB_SUCCESS) {
				std::ostringstream oss;
            	oss << "littleb getBleCharacteristicByCharacteristicPath for path " << path << " failed";
            	throw std::runtime_error(oss.str());
			}
			charecteristic = new BleCharactersitic(*bleCharNew);
		}

		/**
		 * Populate ble_char with characteristic found by using it's uuid
		 *
		 * @param uuid to search for
		 * @param ble_char to populate with characteristic found
		 * @return Result of operation
		 */
		void getBleCharacteristicByUuid(std::string uuid, BleCharactersitic* charecteristic)
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
		 * Populate ble_service with service found by using it's device path under dbus
		 *
		 * @param service_path to search for
		 * @param ble_service to populate with service found
		 * @return Result of operation
		 */
		void getBleServiceByServicePath(std::string path, BleService* service)
		{
			lb_ble_service* bleServiceNew = NULL;
			lb_result_t res = lb_get_ble_service_by_service_path(&m_device, path.c_str(), &bleServiceNew);

			if (bleServiceNew == NULL || res != LB_SUCCESS) {
				std::ostringstream oss;
            	oss << "littleb getBleServiceByServicePath for path " << path << " failed";
            	throw std::runtime_error(oss.str());
			}

			service = new BleService();
			service->initFromStruct(*bleServiceNew);
		}

		/**
		 * Populate ble_service with service found by using it's uuid
		 *
		 * @param bl_dev to search the service in
		 * @param uuid to search for
		 * @param ble_service to populate with service found
		 * @return Result of operation
		 */
		void getBleServiceByUuid(std::string uuid, BleService* service)
		{
			lb_ble_service* bleServiceNew = NULL;
			lb_result_t res = lb_get_ble_service_by_uuid(&m_device, uuid.c_str(), &bleServiceNew);
			
			if (bleServiceNew == NULL || res != LB_SUCCESS) {
				std::ostringstream oss;
            	oss << "littleb getBleServiceByUuid for uuid " << uuid << " failed";
            	throw std::runtime_error(oss.str());
			}

			service = new BleService();
			service->initFromStruct(*bleServiceNew);
		}

		/**
		 * Populate the BLE device with it's services
		 *
		 * @param bl_dev to scan services
		 * @return Result of operation
		 */
		Result getBleDeviceServices()
		{
			return (Result)lb_get_ble_device_services(&m_device);
		}

		/**
		 * Populate the BLE device with it's services
		 *
		 * @param bl_dev to scan services
		 * @return Result of operation
		 */
		Result updateBleDeviceServices()
		{
			return (Result) lb_get_ble_device_services(&m_device);
		}

		// @todo

		/**
		 * Write to a specific BLE device characteristic using it's uuid
		 *
		 * @param dev BLE device to search the characteristic in
		 * @param uuid of the characteristic to write to
		 * @param size of the uint8 array to be written
		 * @param the array of byte buffer to write to the characteristic
		 * @return Result of operation
		 */
		Result writeToCharacteristic(std::string uuid, int size, uint8_t* value)
		{
			return (Result)lb_write_to_characteristic(&m_device, uuid.c_str(), size, value);
		}

		/**
		 * Read from a specific BLE device characteristic using it's uuid
		 *
		 * @param dev BLE device to search the characteristic in
		 * @param uuid of the characteristic to read from
		 * @param size of the uint8 array that was read
		 * @param the array of byte buffer that was read
		 * @return Result of operation
		 */
		Result readFromCharacteristic(std::string uuid, size_t* size, uint8_t** result)
		{
			return (Result) lb_read_from_characteristic(&m_device, uuid.c_str(), size, result);
		}

		//@ todo wrapper to lb_register_characteristic_read_event & lb_register_change_state_event

		
	private:
		lb_bl_device m_device;
		Device(); 
	};
}