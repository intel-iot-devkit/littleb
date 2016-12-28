/*
 * Author: Shiran Ben-Melech <shiran.ben-melech@intel.com>
 * Copyright © 2016 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#pragma once

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <systemd/sd-bus.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * LB return codes
 */
typedef enum {
    LB_SUCCESS = 0,                       /**< Expected response */
    LB_ERROR_FEATURE_NOT_IMPLEMENTED = 1, /**< Feature TODO */
    LB_ERROR_FEATURE_NOT_SUPPORTED = 2,   /**< Feature not supported by HW */
    LB_ERROR_INVALID_CONTEXT = 3,         /**< Not a littleb context*/
    LB_ERROR_INVALID_DEVICE = 4,          /**< Not a BL or BLE device */
    LB_ERROR_INVALID_BUS = 5,             /**< sd bus invalid */
    LB_ERROR_NO_RESOURCES = 6,            /**< No resource of that type avail */
    LB_ERROR_MEMEORY_ALLOCATION = 7,      /**< Memory allocation fail */
    LB_ERROR_SD_BUS_CALL_FAIL = 8,        /**< sd_bus call failure */

    LB_ERROR_UNSPECIFIED = 99 /**< Unknown Error */
} lb_result_t;

typedef struct ble_characteristic {
    const char* char_path; /**< device path under dbus */
    const char* uuid;      /**< uuid of the characteristic. */
} lb_ble_char;

typedef struct ble_service {
    const char* service_path;      /**< device path under dbus */
    const char* uuid;              /**< uuid of the service. */
    bool primary;                  /**< is the service primary in the device */
    lb_ble_char** characteristics; /**< list of the characteristics inside the service */
    int characteristics_size;      /**< count of characteristics in the service */
} lb_ble_service;

typedef struct bl_device {
    const char* device_path;   /**< device path under dbus */
    const char* address;       /**< address of the bluetooth device */
    const char* name;          /**< name of the bluetooth device */
    lb_ble_service** services; /**< list of the service inside the device */
    int services_size;         /**< count of services in the device */
} lb_bl_device;

typedef struct bl_properties {
    bool paired;               /**< is device paired */
    bool trusted;              /**< is device trusted */
    bool connected;            /**< deviced connected */
} lb_bl_properties;

/**
 * Enum used for callback, once registered to get notifications about specific device state change
 *  lb_bl_property_change_notification indicates which event triggered the callback
 */
typedef enum bl_property_change_notification {
  LB_DEVICE_PAIR_EVENT = 0,        /**< device paired */    
  LB_DEVICE_UNPAIR_EVENT = 1 ,     /**< device unpaired */    
  LB_DEVICE_TRUSTED_EVENT = 2,     /**< device trusted */    
  LB_DEVICE_UNTRUSTED_EVENT = 3,   /**< device not trusted */    
  LB_DEVICE_CONNECT_EVENT = 4,     /**< device connected */    
  LB_DEVICE_DISCONNECT_EVENT = 5,  /**< device disconnected */    
  LB_OTHER_EVENT = 6               /**< state change not todo with options listed above */    
} lb_bl_property_change_notification;

/*
* callback type to be used in lb_register_change_state_event
*/
typedef int (*property_change_callback_func)(lb_bl_property_change_notification, void* userdata);

/**
 * Initialize littleb.
 *
 * Set event and event loop configuration
 *
 * @return Result of operation
 */
lb_result_t lb_init();

/**
 * Destroy littleb
 *
 * @return Result of operation
 */
lb_result_t lb_destroy();

/**
 * Populate internal list of bl devices found in a scan of specified length
 *
 * @param seconds to perform device scan
 * @return Result of operation
 */
lb_result_t lb_get_bl_devices(int seconds);

/**
 * Connect to a specific bluetooth device
 *
 * lb_bl_device can be found by name, path or address using lb_get_device functions
 *
 * @param lb_bl_device to connect to
 * @return Result of operation
 */
lb_result_t lb_connect_device(lb_bl_device* dev);

/**
 * Disconnect from a specific bluetooth device
 *
 * @param lb_bl_device to disconnect from
 * @return Result of operation
 */
lb_result_t lb_disconnect_device(lb_bl_device* dev);

/**
 * Pair with specific bluetooth device
 *
 * @param lb_bl_device to pair with
 * @return Result of operation
 */
lb_result_t lb_pair_device(lb_bl_device* dev);

/**
 * Cancel pairing with specific bluetooth device
 *
 * @param lb_bl_device to cancel pair with
 * @return Result of operation
 */
lb_result_t lb_unpair_device(lb_bl_device* dev);

/**
 * Populate ble_char with characteristic found by using it's device path under dbus
 *
 * @param bl_dev to search the characteristic in
 * @param characteristic_path to search for
 * @param ble_char to populate with characteristic found
 * @return Result of operation
 */
lb_result_t lb_get_ble_characteristic_by_characteristic_path(lb_bl_device* dev,
                                                             const char* characteristic_path,
                                                             lb_ble_char** ble_characteristic_ret);

/**
 * Populate ble_char with characteristic found by using it's uuid
 *
 * @param bl_dev to search the characteristic in
 * @param uuid to search for
 * @param ble_char to populate with characteristic found
 * @return Result of operation
 */
lb_result_t lb_get_ble_characteristic_by_uuid(lb_bl_device* dev,
                                              const char* uuid,
                                              lb_ble_char** ble_characteristic_ret);

/**
 * Populate ble_service with service found by using it's device path under dbus
 *
 * @param bl_dev to search the service in
 * @param service_path to search for
 * @param ble_service to populate with service found
 * @return Result of operation
 */
lb_result_t lb_get_ble_service_by_service_path(lb_bl_device* dev,
                                               const char* service_path,
                                               lb_ble_service** ble_service_ret);

/**
 * Populate ble_service with service found by using it's uuid
 *
 * @param bl_dev to search the service in
 * @param uuid to search for
 * @param ble_service to populate with service found
 * @return Result of operation
 */
lb_result_t lb_get_ble_service_by_uuid(lb_bl_device* dev,
                                       const char* uuid,
                                       lb_ble_service** ble_service_ret);

/**
 * Populate the BLE device with it's services
 *
 * @param bl_dev to scan services
 * @return Result of operation
 */
lb_result_t lb_get_ble_device_services(lb_bl_device* dev);

/**
 * Get bluetooth device by using it's device path under dbus
 *
 * @param device_path to search for
 * @param lb_bl_device_ret to populate with the found device
 * @return Result of operation
 */
lb_result_t lb_get_device_by_device_path(const char* device_path, lb_bl_device** bl_device_ret);

/**
 * Get bluetooth device by searching for specific name
 *
 * Will return the first found device with the name specified
 *
 * @param name to search for
 * @param lb_bl_device_ret to populate with the found device
 * @return Result of operation
 */
lb_result_t lb_get_device_by_device_name(const char* name, lb_bl_device** bl_device_ret);

/**
 * Get bluetooth device by searching for specific address
 *
 * @param address to search for
 * @param lb_bl_device_ret to populate with the found device
 * @return Result of operation
 */
lb_result_t lb_get_device_by_device_address(const char* address, lb_bl_device** bl_device_ret);

/**
 * Get device properties, identidy device using address
 *
 * @param device address
 * @param bl_properties_ret to populate with device properties (status)
 * @return Result of operation
 */
lb_result_t lb_get_device_properties(const char* address, lb_bl_properties* bl_properties_ret);

/**
 * Write to a specific BLE device characteristic using it's uuid
 *
 * @param dev BLE device to search the characteristic in
 * @param uuid of the characteristic to write to
 * @param size of the uint8 array to be written
 * @param the array of byte buffer to write to the characteristic
 * @return Result of operation
 */
lb_result_t
lb_write_to_characteristic(lb_bl_device* dev, const char* uuid, int size, uint8_t* value);

/**
 * Read from a specific BLE device characteristic using it's uuid
 *
 * @param dev BLE device to search the characteristic in
 * @param uuid of the characteristic to read from
 * @param size of the uint8 array that was read
 * @param the array of byte buffer that was read
 * @return Result of operation
 */
lb_result_t
lb_read_from_characteristic(lb_bl_device* dev, const char* uuid, size_t* size, uint8_t** result);

/**
 * Register a callback function for an event of characteristic value change
 *
 * @param dev BLE device to search the characteristic in
 * @param uuid of the characteristic to read from
 * @param callback function to be called when char value changed
 * @param userdata to pass in the callback function
 * @return Result of operation
 */
lb_result_t lb_register_characteristic_read_event(lb_bl_device* dev,
                                                  const char* uuid,
                                                  sd_bus_message_handler_t callback,
                                                  void* userdata);

/**
 * Register a callback function for device state (connected/paired/trusted) change event
 *
 * @param dev BLE device to get notifications for
 * @param callback function to be called when state change
 * @param userdata to pass in the callback function
 * @return Result of operation
 */
lb_result_t lb_register_change_state_event(lb_bl_device* dev,
                                           property_change_callback_func callback,
                                           void* userdata);

/**
 * Special function to parse uart tx line buffer
 *
 * @param message sd_bus_message to prase the buffer array from
 * @param result buffer to accommodate the result
 * @param size of the buffer
 * @return Result of operation
 */
lb_result_t lb_parse_uart_service_message(sd_bus_message* message, const void** result, size_t* size);


#ifdef __cplusplus
}
#endif
