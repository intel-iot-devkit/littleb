import sys
path_to_lib = "../build/wrappers/python_swig/python2"
sys.path.append(path_to_lib)
import littleb as lb
import time


def firmata_callback_py(message, error):
	print "\nin firmata_callback in python!"
	print "Data: " + str(message)
	return 1

def change_state_callback_py(state_changed):
	print "\nin change_state_callback in python!"
	print "State Changed: " + str(state_changed)
	return 1


IR_TEMP_DATA_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
IR_TEMP_CONFIG_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
DEVICE = "FIRMATA"
LED_ON = lb.uintVector(3)
LED_ON[0] = 0x91
LED_ON[1] = 0x20
LED_ON[2] = 0x00
LED_OFF = lb.uintVector(3) 
LED_OFF[0] = 0x91
LED_OFF[1] = 0x00
LED_OFF[2] = 0x00

dev = None
try:
	
	dev_manager = lb.DeviceManager.getInstance()
	dev_manager.getBlDevices(10)
	print "Getting Device " + DEVICE
	dev = dev_manager.getDeviceByName(DEVICE)
	dev.connect()
	print "Connected to: " + dev.getName() + "\nDevice MAC address: " + str(dev.getAddress())
	services = dev.getBleDeviceServices()
	
	if not services:
		# print services
		raise Exception("No services found") 
	else:
		print "Services found: "

		for i in range(0, dev.getNumOfServices()):
			
			print str(services[i].getPath()) + "\t" + str(services[i].getUuid())
			print "Characteristics Found: "

			characteristics = services[i].getCharacteristics()
			for j in range(0, characteristics.size()):
				print str(characteristics[j].getPath()) + "\t" + str(characteristics[j].getUuid())


	print "Blinking"
	for i in range(0,10):

		dev.writeToCharacteristic(IR_TEMP_CONFIG_UUID, 3, LED_ON)
		time.sleep(0.5)

		dev.writeToCharacteristic(IR_TEMP_CONFIG_UUID, 3, LED_OFF)
		time.sleep(0.5)

	lb.registerCallbackEventRead(dev, IR_TEMP_DATA_UUID, firmata_callback_py)
	lb.registerCallbackChangeState(dev, change_state_callback_py)
	print "get_version"
	get_version = lb.uintVector(3) 
	get_version[0] = 0xf0
	get_version[1] = 0x79
	get_version[2] = 0xf7

	dev.writeToCharacteristic(IR_TEMP_CONFIG_UUID, 3, get_version)

	print "waiting for callbacks"
	time.sleep(2)

	# read device properties
	firmataProperties = dev.getDeviceProperties()
	print "Firmata state: Connected: " + str(firmataProperties.connected) + ". Paired: " + str(firmataProperties.paired) + ". Trusted: " + str(firmataProperties.trusted)

	dev.disconnect()
	print "Firmata disconnected"
	time.sleep(5)
except Exception as e:
	print "EXCEPTION: " + str(e)
	if dev:
		dev.disconnect()
		print "Firmata disconnected"
	



