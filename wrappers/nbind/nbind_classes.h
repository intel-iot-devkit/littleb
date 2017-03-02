#include "nbind/nbind.h"

NBIND_GLOBAL() {
  function(parseUartServiceMessage);
  function(parseDBusMessage);
}
NBIND_CLASS(sd_bus_message) {}
NBIND_CLASS(sd_bus_error) {}
NBIND_CLASS(BleCharactersitic) {
  construct<>();
  construct<std::string, std::string>();
  // construct<lb_ble_char&>();
  method(getPath);
  method(getUuid);
  
}
NBIND_CLASS(BleService) {
  construct<>();
  method(getPath);
  method(getUuid);
  method(getPrimary);
  method(getCharacteristics);

}
NBIND_CLASS(Device) {
  construct<lb_bl_device*>();
  method(getName);
  method(getAddress);
  method(connect);
  method(disconnect);
  method(getBleDeviceServices);
  method(getNumOfServices);
  method(getDeviceProperties);
  method(getBleCharacteristicByPath);
  method(getBleCharacteristicByUuid);
  method(getBleServiceByPath);
  method(getBleServiceByUuid);
  method(writeToCharacteristic);
  method(readFromCharacteristic);
  method(registerCallbackReadEvent); 
  method(registerCallbackStateEvent); 

}

NBIND_CLASS(DeviceManager) {
  method(getInstance);
  method(getBlDevices);
  method(getDeviceByName);
}
