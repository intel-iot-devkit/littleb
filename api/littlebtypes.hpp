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

    class BleCharactersitic
    {
    public:
        BleCharactersitic()
        : m_path("")  , m_uuid("")
        {}

        BleCharactersitic(std::string path, std::string uuid)
        : m_path(path)  , m_uuid(uuid)
        {}

        BleCharactersitic(lb_ble_char& structChar)
        {
            m_path = structChar.char_path;
            m_uuid = structChar.uuid;
        }

        ~BleCharactersitic(){}

        std::string getPath() {return m_path;}
        std::string getUuid() {return m_uuid;}

        void setPath(std::string path) {m_path = path;}
        void setUuid(std::string uuid) {m_uuid = uuid;}

    private:
       std::string m_path;
       std::string m_uuid;
    };

    class BleService
    {
    public:
        BleService() {}
        ~BleService() {} //@todo
        
        bool initFromStruct(ble_service& bleService)
        {
            bool initOk = true;
            path = bleService.service_path;
            uuid = bleService.uuid;
            primary = bleService.primary;

            for (int i = 0; i < bleService.characteristics_size; ++i)
            {
                if (bleService.characteristics[i] != NULL) {
                    BleCharactersitic* characteristic = new BleCharactersitic(*characteristics[i]);
                    characteristics.push_back(characteristic);
                } else {
                    initOk = false;
                    // @todo dodgy data
                }          
            }

            return initOk;      
        }
    
    private:
        bool primary;
        std::string path;
        std::string uuid;
        std::vector<BleCharactersitic*> characteristics ;     
    };
}