#pragma once

#include "littlebtypes.h"

#ifdef BUILDING_NODE_EXTENSION
    #include "nbind/api.h"
    
    struct r_call_event {
        v8::Persistent<v8::Function> r_call;
    };

#endif


class Device
{
  public:

    Device(lb_bl_device* device);
	
    ~Device();

    void connect();

    void disconnect();

    void pair();

    void unpair();

    std::shared_ptr<BleCharactersitic> getBleCharacteristicByPath(std::string path);

    std::shared_ptr<BleCharactersitic> getBleCharacteristicByUuid(std::string uuid);

    std::shared_ptr<BleService> getBleServiceByPath(std::string path);

    std::shared_ptr<BleService> getBleServiceByUuid(std::string uuid);

    std::vector<std::shared_ptr<BleService>> getBleDeviceServices();

    void writeToCharacteristic(std::string uuid, int size, std::vector<uint8_t> value);

    std::vector<uint8_t> readFromCharacteristic(std::string uuid);

    BlProperties getDeviceProperties();

    void registerChangeStateEvent(int (*propertyChangeCallback)(lb_bl_property_change_notification bpcn, void* userdata), void* userdata);

    std::string getName();

    std::string getAddress();

    int getNumOfServices();

    void registerCharacteristicReadEvent(std::string uuid, int (*call_func)(sd_bus_message*, void*,sd_bus_error*), void* userdata);
#ifdef BUILDING_WITH_SWIG
    void registerCallbackReadEvent(std::string uuid, void* python_func_ptr);
    void registerCallbackStateEvent(void* python_func_ptr);
#endif

#ifdef BUILDING_NODE_EXTENSION
    void registerCallbackReadEvent(std::string uuid, nbind::cbFunction& cbFunc);
    void registerCallbackStateEvent(nbind::cbFunction& cbFunc);
    std::vector<std::shared_ptr<r_call_event>> r_call_events;
#endif
  private:
    lb_bl_device m_device;
    std::vector<std::shared_ptr<BleService>> v_services;
    BlProperties prop;
    Device();

};
