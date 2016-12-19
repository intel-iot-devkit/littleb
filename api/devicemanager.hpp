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

#include "littlebtypes.hpp"


namespace littleb
{
	class DeviceManager
	{
	public:		

		static DeviceManager& getInstance()
		{
			static DeviceManager instance;
			return instance;
		}

		~DeviceManager()
		{
			lb_destroy();
		} 

		
		Result getBlDevices(int seconds)
		{
			return (Result)lb_get_bl_devices(seconds);
		}
		

		Result getDeviceByName(std::string name, Device* out)
		{
			bl_device *device;
			Result res = (Result)lb_get_device_by_device_name(name.c_str(), &device);
			out = new Device(device);
		}


    	Result ScanDevices(int seconds = 5)
		{
    		return (Result)lb_get_bl_devices(seconds);
        }

	private:
		DeviceManager()	
		{
			if(lb_init() != LB_SUCCESS) {
				 throw std::invalid_argument("Error initialising DeviceManager");
			}
		}	
	};
}
