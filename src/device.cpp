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

//#pragma once


#include <sstream>
#include <stdexcept>
#include <iostream>
#include "device.h"
#ifdef BUILDING_WITH_SWIG
#include "python_cpp_cb_connect.hpp"
#endif
#ifdef BUILDING_NODE_EXTENSION
#include "v8_cpp_connect.hpp"
#endif
// using namespace littleb;
/**
 * API to device
 *
 */

    /**
     * Constructor
     *
     * Constructor being used by DeviceManager generate and return device
     * Cannot call an empty constructor
     *
     * @throws std::invalid_argument in case of null input
     */
    Device::Device(lb_bl_device* device)
    {
        if (device == NULL) {
            throw std::invalid_argument("Error initialising Device");
        } else {
            memcpy(&m_device, device, sizeof(lb_bl_device));
           
        }
    }

    /**
     * Destructor
     */
    Device::~Device()
    {
       
    }


    /**
     * Connect device
     *
     * @throws std::runtime_error when lb_connect_device() exit with an error
    */
    void
    Device::connect()
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
    Device::disconnect()
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
    Device::pair()
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
    Device::unpair()
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
    std::shared_ptr<BleCharactersitic> 
    Device::getBleCharacteristicByPath(std::string path)
    {
        lb_ble_char* bleCharNew = NULL;
        lb_result_t res = lb_get_ble_characteristic_by_characteristic_path(&m_device, path.c_str(), &bleCharNew);
        if (res != LB_SUCCESS) {
            std::ostringstream oss;
            oss << "littleb getBleCharacteristicByCharacteristicPath for path " << path
                << " failed";
            throw std::runtime_error(oss.str());
        }
        return std::make_shared<BleCharactersitic>(*bleCharNew);

    }

    /**
     * Populate BleCharactersitic with characteristic found by using it's uuid
     *
     * @param uuid to search for
     *  to populate with characteristic found
     * @throws std::runtime_error when get characteristic call exit with an error
     */
    std::shared_ptr<BleCharactersitic> 
    Device::getBleCharacteristicByUuid(std::string uuid)
    {
        lb_ble_char* bleCharNew = NULL;
        lb_result_t res = lb_get_ble_characteristic_by_uuid(&m_device, uuid.c_str(), &bleCharNew);

        if (bleCharNew == NULL || res != LB_SUCCESS) {
            std::ostringstream oss;
            oss << "littleb getBleCharacteristicByUuid for uuid " << uuid << " failed";
            throw std::runtime_error(oss.str());
        }
        
        return std::make_shared<BleCharactersitic>(*bleCharNew);
    }

    /**
     * Populate BleService with service found by using it's device path under dbus
     *
     * @param service path to search for
     * @param BleService to populate with service found
     * @throws std::runtime_error when get service call exit with an error or returned a null
     * service
     */
    std::shared_ptr<BleService> 
    Device::getBleServiceByPath(std::string path)
    {

        lb_ble_service* bleServiceNew = NULL;
        lb_result_t res = lb_get_ble_service_by_service_path(&m_device, path.c_str(), &bleServiceNew);

        if (bleServiceNew == NULL || res != LB_SUCCESS) {
            std::ostringstream oss;
            oss << "littleb getBleServiceByPath for path " << path << " failed";
            throw std::runtime_error(oss.str());
        }

        auto out = std::make_shared<BleService>();
        out->init(*bleServiceNew);
        return out;
    }

    /**
     * Populate BleService with service found by using it's uuid
     *
     * @param uuid to search for
     * @param BleService to populate with service found
     * @throws std::runtime_error when get service call exit with an error or returned a null
     * service
     */
    std::shared_ptr<BleService> 
    Device::getBleServiceByUuid(std::string uuid)
    {
        lb_ble_service* bleServiceNew = NULL;
        lb_result_t res = lb_get_ble_service_by_uuid(&m_device, uuid.c_str(), &bleServiceNew);

        if (bleServiceNew == NULL || res != LB_SUCCESS) {
            std::ostringstream oss;
            oss << "littleb getBleServiceByUuid for uuid " << uuid << " failed";
            throw std::runtime_error(oss.str());
        }

        auto out = std::make_shared<BleService>();
        out->init(*bleServiceNew);
        return out;
    }

    /**
     * Populate the BLE device with it's services
     * @throws std::runtime_error when get device services call exit with an error
     *
     */
    std::vector<std::shared_ptr<BleService>>
    Device::getBleDeviceServices()
    {
        if (lb_get_ble_device_services(&m_device) != LB_SUCCESS) {
            throw std::runtime_error("littleb device getBleDeviceServices call failed");
        }
        v_services.clear();
        if (m_device.services_size > 0) {          
             
            for (int i = 0; i < m_device.services_size; ++i) {
                auto temp = std::make_shared<BleService>();
                temp->init(*m_device.services[i]);
                v_services.push_back(temp);
            }
        }

        return v_services;
    }

    /**
     * Write to a specific BLE device characteristic using it's uuid
     *
     * @param uuid of the characteristic to write to
     * @param size of the uint8 array to be written
     * @param the array of byte buffer to write to the characteristic
     * @throws std::runtime_error when lb_write_to_characteristic() call exit with an error
     */
    void Device::writeToCharacteristic(std::string uuid, int size, std::vector<uint8_t> value)
    {
        
        if (lb_write_to_characteristic(&m_device, uuid.c_str(), size, &value[0])!= LB_SUCCESS) {
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
    std::vector<uint8_t>
    Device::readFromCharacteristic(std::string uuid)
    {
        size_t* buf_length = 0;
        uint8_t buf[1024];
        uint8_t* buf_ptr = buf;   
        
        if (lb_read_from_characteristic(&m_device, uuid.c_str(), buf_length, &buf_ptr)) {
            throw std::runtime_error("littleb device readFromCharacteristic call failed");
        }
        
        std::vector<uint8_t> res(buf_ptr, buf_ptr + *buf_length);
        return res;
    }

    /**
     * Get device properties
     *
     * @throws std::runtime_error when lb_get_device_properties() call exit with an error
     * @return device properties
     */
    BlProperties
    Device::getDeviceProperties()
    {
        lb_bl_properties tempProps;

        if (lb_get_device_properties(m_device.address, &tempProps) != LB_SUCCESS) {
            throw std::runtime_error("littleb device getDeviceProperties call failed");
        }

        prop.trusted = tempProps.trusted;
        prop.paired = tempProps.paired;
        prop.connected = tempProps.connected;
        return prop;
    }

/**
     * Register a callback function for an event of characteristic value change
     *
     * @param uuid of the characteristic to read from
     * @param callback function to be called when char value changed
     *        typedef int (*sd_bus_message_handler_t)(sd_bus *bus, int ret, sd_bus_message *m, void
     * *userdata);
     * @param userdata to pass in the callback function
     *
     * @throws std::runtime_error when lb_register_characteristic_read_event exit with an error
     */
    void Device::registerCharacteristicReadEvent(
        std::string uuid, int (*call_func)(sd_bus_message*, void* ,sd_bus_error*), void* userdata){

        sd_bus_message_handler_t callback = (sd_bus_message_handler_t) call_func;
        if (lb_register_characteristic_read_event(&m_device, uuid.c_str(), callback,  userdata) != LB_SUCCESS) {
            std::ostringstream oss;
            oss << "littleb registerCharacteristicReadEvent for uuid " << uuid << " failed";
            throw std::runtime_error(oss.str());
        }
        
    }

    /**
     * Register a callback function for device state (connected/paired/trusted) change event
     *
     * @param callback function to be called when state change
     * @param userdata to pass in the callback function
     *
     * @throws std::runtime_error when lb_register_change_state_event exit with an error
     */
    void
    Device::registerChangeStateEvent(int (*call_back)(lb_bl_property_change_notification bpcn, void* userdata), void* userdata)
    {
        property_change_callback_func callback = (property_change_callback_func) call_back;

        if (lb_register_change_state_event(&m_device, callback, userdata) != LB_SUCCESS) {
            throw std::runtime_error("littleb device registerChangeStateEvent call failed");
        }
    }

    

    

    /**
     * Get device name
     */
    std::string
    Device::getName()
    {
        return m_device.name;
    }

    /**
     * Get device address
    */
    std::string
    Device::getAddress()
    {
        return m_device.address;
    }

    /**
     * Get number of services
    */
    int
    Device::getNumOfServices()
    {
        return m_device.services_size;
    }

    Device::Device(){}
#ifdef BUILDING_WITH_SWIG

    void Device::registerCallbackReadEvent(std::string uuid, void* python_func_ptr) {
        registerCharacteristicReadEvent(uuid, cbReadEvent, python_func_ptr);
    }
    void Device::registerCallbackStateEvent(void* python_func_ptr) {
        registerChangeStateEvent(cbChangeState, python_func_ptr);
    }
#endif
#ifdef BUILDING_NODE_EXTENSION
    void Device::registerCallbackReadEvent(std::string uuid, nbind::cbFunction& cbFunc) {

        v8::Isolate* isolate = v8::Isolate::New(v8::Isolate::CreateParams());
        v8::Local<v8::Function> func = cbFunc.getJsFunction();
        r_call_event.Reset(isolate, func);
        registerCharacteristicReadEvent(uuid, uvworkReadEvent, this);
    }

    void Device::registerCallbackStateEvent(nbind::cbFunction& cbFunc){

        v8::Isolate* isolate = v8::Isolate::New(v8::Isolate::CreateParams());
        v8::Local<v8::Function> func = cbFunc.getJsFunction();     
        r_call.Reset(isolate, func);
        registerChangeStateEvent(uvworkStateChange, this);
    
    }
#endif
