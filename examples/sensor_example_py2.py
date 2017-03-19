import sys
path_to_lib = "../build/wrappers/python_swig/python2"
sys.path.append(path_to_lib)
import littleb as lb
import time
def sensor_callback_py(message, error):
	print "\nin sensor_callback in python!"
	print "Data: " + str(message)
	return 1
IR_TEMP_DATA_UUID = "f000aa01-0451-4000-b000-000000000000"
IR_TEMP_CONFIG_UUID = "f000aa02-0451-4000-b000-000000000000"

DEVICE = "CC2650 SensorTag"

dev = None
try:

	print "Scanning..."
	dev_manager = lb.DeviceManager.getInstance()
	dev_manager.getBlDevices(10)
	print "Getting Device " + DEVICE
	dev = dev_manager.getDeviceByName(DEVICE)
	dev.connect()
	print "Connected to: " + dev.getName() + "\nDevice MAC address: " + str(dev.getAddress())
	
	
	services = dev.getBleDeviceServices()
	
	if not services:
		raise Exception("No services found") 
	else:
		print "Services found: "

		for i in range(0, dev.getNumOfServices()):
			
			print str(services[i].getPath()) + "\t" + str(services[i].getUuid())
			print "Characteristics Found: "

			characteristics = services[i].getCharacteristics()
			for j in range(0, characteristics.size()):
				print str(characteristics[j].getPath()) +"\t" + str(characteristics[j].getUuid())            
	flagOn = lb.uintVector(1)
	flagOn[0] = 0x01
	print "Enable IR Temperature Sensor"
	dev.writeToCharacteristic(IR_TEMP_CONFIG_UUID, 1, flagOn)
	
	# userdata = "temperature sensor test"
	lb.registerCallbackEventRead(dev, IR_TEMP_DATA_UUID, sensor_callback_py)
	
	print "waiting for callbacks"

	dev.disconnect()
	print "Sensor disconnected"
	time.sleep(5)
except Exception as e:
	print "Exception: " + str(e)
	if dev:
		dev.disconnect()
		print "Sensor disconnected"




