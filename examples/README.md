
# littleb use examples
you can find here few basic examples that use littleb to communicate to sevral IOT devices 
and demonstrate simple use cases for it.

## hello_littleb
Uses Arduino 101 flashed with StandardFirmataBLE
(https://github.com/firmata/arduino/tree/master/examples/StandardFirmataBLE)
It is looking for a bluetooth device named "FIRMATA" and will connect to it and make the on board LED blink

## littleb_notifications_example
Uses Arduino 101 flashed with StandardFirmataBLE
It is looking for a bluetooth device named "FIRMATA" and will connect to it 
register to recive notification on properties changes on device, like disconnect etc.

## littleb_sensor_tag_example
Uses Texas Instruments sensor tag
Looking for a bluetooth device named "CC2650 SensorTag"
Reads tempreture from the device and displays it 

## littleb_heartrate_example
Uses Arduino flashed with Zephyr sample peripheral_hr
(https://www.zephyrproject.org/doc/samples/bluetooth/peripheral_hr/README.html)
instruction as to how to flush an Arduino device with Zephyer sample can be found here:
(https://www.zephyrproject.org/doc/getting_started/getting_started.html)
in the below link, make sure you read the "Flashing the Bluetooth Core" section
(https://www.zephyrproject.org/doc/boards/x86/arduino_101/doc/board.html)

If having trouble reading heartrate of the device check the next below:
Update bluez to version 5.43 and up
Make sure that your DBus policy permits users to access heartrate interface. 
If needed, add the following lines to /etc/dbus-1/system.d/bluetooth.conf 
    <allow send_interface="org.bluez.HeartRateWatcher1"/>
    <allow send_interface="org.bluez.HeartRateManager1"/>

## littleb_base_example
Uses Arduino 101 flashed with StandardFirmataBLE
This sample is written in c++, uses the littleb c++ wrappers. 
Same as hello_littleb example
