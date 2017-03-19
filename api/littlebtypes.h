#pragma once
#include "littleb.h"
#include <string>
#include <vector>
#include <stdexcept>
#include <memory>
#include <sstream>
#include <iostream>

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





class BleCharactersitic
{
  public:
    BleCharactersitic();


    BleCharactersitic(std::string path, std::string uuid);


    BleCharactersitic(lb_ble_char& structChar);


    ~BleCharactersitic();


    std::string getPath();

    std::string getUuid();

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
    BleService();

    ~BleService();

    bool init(ble_service& bleService);
    std::string getPath();
    std::string getUuid();

    bool getPrimary();
    std::vector<std::shared_ptr<BleCharactersitic>> getCharacteristics();

  private:
    bool m_primary;
    std::string m_path;
    std::string m_uuid;
    std::vector<std::shared_ptr<BleCharactersitic>> m_characteristics;
};



std::vector<uint8_t> parseUartServiceMessage(sd_bus_message* message);
std::vector<uint8_t> parseDBusMessage(sd_bus_message* message);