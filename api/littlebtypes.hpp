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

#include "littleb.h"
#include <string>
#include <vector>

namespace littleb
{
/**
 * LB return codes
 */
typedef enum {
    SUCCESS = 0,                       /**< Expected response */
    ERROR_FEATURE_NOT_IMPLEMENTED = 1, /**< Feature TODO */
    ERROR_FEATURE_NOT_SUPPORTED = 2,   /**< Feature not supported by HW */
    ERROR_INVALID_CONTEXT = 3,         /**< Not a littleb context*/
    ERROR_INVALID_DEVICE = 4,          /**< Not a BL or BLE device */
    ERROR_INVALID_BUS = 5,             /**< sd bus invalid */
    ERROR_NO_RESOURCES = 6,            /**< No resource of that type avail */
    ERROR_MEMEORY_ALLOCATION = 7,      /**< Memory allocation fail */
    ERROR_SD_BUS_CALL_FAIL = 8,        /**< sd_bus call failure */

    ERROR_UNSPECIFIED = 99 /**< Unknown Error */
} Result;

/**
 * Device key properties
 */
struct BlProperties {
    bool paired;    /**< is device paired */
    bool trusted;   /**< is device trusted */
    bool connected; /**< deviced connected */
};

/**
 * Enum used for callback, once registered to get notifications about specific device state change
 *  lb_bl_property_change_notification indicates which event triggered the callback
 */
enum BlPropertyChangeNotification {
    DEVICE_PAIR_EVENT = 0,       /**< device paired */
    DEVICE_UNPAIR_EVENT = 1,     /**< device unpaired */
    DEVICE_TRUSTED_EVENT = 2,    /**< device trusted */
    DEVICE_UNTRUSTED_EVENT = 3,  /**< device not trusted */
    DEVICE_CONNECT_EVENT = 4,    /**< device connected */
    DEVICE_DISCONNECT_EVENT = 5, /**< device disconnected */
    OTHER_EVENT = 6              /**< state change not todo with options listed above */
};


/*
* callback type to be used in lb_register_change_state_event
*/
typedef int (*propertyChangeCallbackFunc)(BlPropertyChangeNotification, void* userdata);


/**
 * @brief BleCharactersitic represents BLE characteristics
 */
class BleCharactersitic
{
  public:
    BleCharactersitic() : m_path(""), m_uuid("")
    {
    }

    BleCharactersitic(std::string path, std::string uuid) : m_path(path), m_uuid(uuid)
    {
    }

    BleCharactersitic(lb_ble_char& structChar)
    {
        m_path = structChar.char_path;
        m_uuid = structChar.uuid;
    }

    ~BleCharactersitic()
    {
    }

    std::string
    getPath()
    {
        return m_path;
    }
    std::string
    getUuid()
    {
        return m_uuid;
    }

  private:
    std::string m_path;
    std::string m_uuid;
};


/**
 * @brief BleService represents BLE service
 */
class BleService
{
  public:
    BleService()
    {
    }

    ~BleService()
    {
        while (!m_characteristics.empty()) {
            // triggering the vector objects desructor
            m_characteristics.pop_back();
        }
    }

    bool
    init(ble_service& bleService)
    {
        bool initOk = true;
        m_path = bleService.service_path;
        m_uuid = bleService.uuid;
        m_primary = bleService.primary;

        for (int i = 0; i < bleService.characteristics_size; ++i) {
            if (bleService.characteristics[i] != NULL) {
                BleCharactersitic* characteristic = new BleCharactersitic(*bleService.characteristics[i]);
                m_characteristics.push_back(characteristic);
            } else {
                initOk = false;
            }
        }

        return initOk;
    }

    std::string
    getPath()
    {
        return m_path;
    }
    std::string
    getUuid()
    {
        return m_uuid;
    }
    bool
    getPrimary()
    {
        return m_primary;
    }

    const std::vector<BleCharactersitic*>&
    getCharacteristics()
    {
        return m_characteristics;
    }


  private:
    bool m_primary;
    std::string m_path;
    std::string m_uuid;
    std::vector<BleCharactersitic*> m_characteristics;
};
}