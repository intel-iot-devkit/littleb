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



#include "littlebtypes.h"


/**
 * @brief BleCharactersitic represents BLE characteristics
 */

BleCharactersitic::BleCharactersitic() : m_path(""), m_uuid("")
{
}

BleCharactersitic::BleCharactersitic(std::string path, std::string uuid) : m_path(path), m_uuid(uuid)
{
}

BleCharactersitic::BleCharactersitic(lb_ble_char& structChar)
{
    m_path = structChar.char_path;
    m_uuid = structChar.uuid;
}

BleCharactersitic::~BleCharactersitic()
{
}

std::string
BleCharactersitic::getPath()
{
    return m_path;
}
std::string
BleCharactersitic::getUuid()
{
    return m_uuid;
}


BleService::BleService()
{
}

BleService::~BleService()
{
    while (!m_characteristics.empty()) {
                // triggering the vector objects desructor
        m_characteristics.pop_back();
    }
}

bool
BleService::init(ble_service& bleService)
{
    bool initOk = true;
    m_path = bleService.service_path;
    m_uuid = bleService.uuid;
    m_primary = bleService.primary;

    for (int i = 0; i < bleService.characteristics_size; ++i) {
        if (bleService.characteristics[i] != NULL) {
            auto characteristic =  std::make_shared<BleCharactersitic>(*bleService.characteristics[i]);
            m_characteristics.push_back(characteristic);
        } else {
            initOk = false;
        }
    }

    return initOk;
}

std::string
BleService::getPath()
{
    return m_path;
}
std::string
BleService::getUuid()
{
    return m_uuid;
}
bool
BleService::getPrimary()
{
    return m_primary;
}

std::vector<std::shared_ptr<BleCharactersitic>>
BleService::getCharacteristics()
{
    return m_characteristics;
}
/**
 * Special function to parse uart to line buffer, basicly a wrapper function to
 * parseDBusMessage
 *
 * @param message sd_bus_message to prase the buffer array from
 * @return result buffer
 *
 * @throws std::runtime_error when lb_parse_uart_service_message exit with an error
 */
std::vector<uint8_t> parseUartServiceMessage(sd_bus_message* message)
{
    size_t size = 0;
    uint8_t* result = NULL;
    std::vector<uint8_t> outResult;

    if (lb_parse_uart_service_message(message, (const void**) &result, &size) != LB_SUCCESS) {
        throw std::runtime_error("littleb device lb_parse_uart_message call failed");
    }

    if (size > 0) {
        // outResult = new uint8_t[size];

        for (size_t i = 0; i < size; i++) {
            outResult.push_back(result[i]);
        }
    }
    return outResult;
}
/**
 * Function call that extract a buffer array from sd_bus_messages
 * Designed for messages of type: "sa{sv}as"
 *
 * @param message sd_bus_message to prase the buffer array from
 * @return result buffer
 * @throws std::runtime_error when lb_register_change_state_event exit with an error
  */
std::vector<uint8_t> parseDBusMessage(sd_bus_message* message)
{
    size_t size = 0;
    uint8_t* result = NULL;
    std::vector<uint8_t> outResult;

    if (lb_parse_dbus_message(message, (const void**) &result, &size) != LB_SUCCESS) {
        throw std::runtime_error("littleb device lb_parse_dbus_message call failed");
    }

    if (size > 0) {
        // outResult = new uint8_t[size];

        for (size_t i = 0; i < size; i++) {
            outResult.push_back(result[i]);
        }
    }
    return outResult;
}