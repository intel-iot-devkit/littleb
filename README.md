LittleB: Bluetooth Low Energy Library
=============

This project aims to create a clean, modern and easy to use Bluetooth Low Energy API.
LittleB exposes the BLE GATT API for Pure C using BlueZ over SD-Bus.

API Documentation
============

WIP

Using LittleB
============

LittleB requires CMake 3.1+ for building. It also
requires BlueZ with GATT profile activated, which is currently experimental (as
of BlueZ 5.37), so you might have to run bluetoothd with the -E flag. For
example, on a system with systemd (Fedora, poky, etc.) edit the
bluetooth.service file (usually found in /usr/lib/systemd/system/ or
/lib/systemd/system) and append -E to ExecStart line, restart the daemon with
systemctl restart bluetooth.

~~~~~~~~~~~~~{.sh}
mkdir build
cd build
cmake ..
make
make install
~~~~~~~~~~~~~

The last command will create the include/ and lib/ directories with a copy of
the headers and library objects respectively in your build location. Note that
doing an out-of-source build may cause issues when rebuilding later on.

Our cmake configure has a number of options, *cmake-gui* or *ccmake* can show
you all the options. The interesting ones are detailed below:

Changing install path from /usr/local to /usr
~~~~~~~~~~~~~
-DCMAKE_INSTALL_PREFIX=/usr
~~~~~~~~~~~~~
Building debug build:
~~~~~~~~~~~~~
-DCMAKE_BUILD_TYPE=DEBUG
~~~~~~~~~~~~~

The hellolittleb example uses Arduino 101 flashed with StandardFirmataBLE
(https://github.com/firmata/arduino/tree/master/examples/StandardFirmataBLE)
It is looking for a bluetooth device named "FIRMATA" and will connect to it and make the on board LED blink. 

Common issues
============

If you have any issues, please go through the [Troubleshooting Guide](TROUBLESHOOTING.md). If the solution is not there, please create a new issue on [Github](https://github.com/intel-iot-devkit/littleb).

Contributing to LittleB
============

You must agree to Developer Certificate of Origin and Sign-off your code,
using a real name and e-mail address. 
Please check the [Contribution](CONTRIBUTING.md) document for more details.
