#pragma once
// #include "littlebtypes.h"
#include "device.h"

class DeviceManager
{
  public:
    static DeviceManager& getInstance();
    void getBlDevices(int seconds = 5);
    std::shared_ptr<Device> getDeviceByName(std::string name);
    ~DeviceManager();

  private:
    DeviceManager();
    DeviceManager(DeviceManager& device);

};
#ifdef BUILDING_NODE_EXTENSION
#include "nbind_classes.h"
#endif




