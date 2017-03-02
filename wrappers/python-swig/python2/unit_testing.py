import littleb as lb
import time
# import numpy
# import ctypes

def firmata_callback_py(message, error):
	print "\nin firmata_callback in python!"
	print "Data: " + str(message)
	return 1

def change_state_callback_py(state_changed):
	print "\nin change_state_callback in python!"
	print "State Changed: " + str(state_changed)
	return 1


SENSOR_DATA_UUID = "f000aa01-0451-4000-b000-000000000000"
SENSOR_CONFIG_UUID = "f000aa02-0451-4000-b000-000000000000"
FIRMATA_DATA_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"

FIRMATA_CONFIG_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
# DEVICE_S = "CC2650 SensorTag"
DEVICE_S = "FIRMATA"
LED_ON = lb.uintVector(3)
LED_ON[0] = 0x91
LED_ON[1] = 0x20
LED_ON[2] = 0x00
LED_OFF = lb.uintVector(3) 
LED_OFF[0] = 0x91
LED_OFF[1] = 0x00
LED_OFF[2] = 0x00
device = None

try:
	
	dev_manager = lb.DeviceManager.getInstance()
	print "Scanning..."
	dev_manager.getBlDevices(6)
	print "Getting Device " + DEVICE_S
	device = dev_manager.getDeviceByName(DEVICE_S)

	print "Connecting Device " + DEVICE_S
	device.connect()
	
	# print "Pairing Devices"
	# device.pair()

	# print "Connected to: " + device.getName() + "\nDevice MAC address: " + str(device.getAddress())
	
	# services = device.getBleDeviceServices()
	services = device.getBleDeviceServices()
	if not services:
		# print services
		raise Exception("No services found") 
	else:
		b = services[0].getPath()
		print "Services found: " + str(device.getNumOfServices())

		for i in range(0, device.getNumOfServices()):
			
			print "Getting BleService By Path..."
			bleService = device.getBleServiceByPath(services[i].getPath())
			print "Service uuid: " + str(bleService.getUuid())

			print "Getting BleService By UUID..."
			bleService2 = device.getBleServiceByUuid(services[i].getUuid())
			print "Service Path: " + str(bleService2.getPath())
			print ' '
			characteristics = services[i].getCharacteristics()
			for j in range(0, characteristics.size()):
				print str(characteristics[j].getPath()) + str(characteristics[j].getUuid())
				print "Getting BleCharacteristic By Path: " + str(characteristics[j].getPath())

				bleCharactersitic = device.getBleCharacteristicByPath(characteristics[j].getPath())            
				if bleCharactersitic:
					print "bleCharactersitic uuid: " + str(bleCharactersitic.getUuid())
				else:
					print characteristics[j].getPath()
				print "Getting BleCharacteristic By uuid..."
				bleCharactersitic2 = device.getBleCharacteristicByUuid(characteristics[j].getUuid())            
				print "bleCharactersitic Path:" + str(bleCharactersitic2.getPath())
				print ' '
	prop = device.getDeviceProperties()
	print "Device state: Connected: " + str(prop.connected) + ". Paired: " + str(prop.paired) + ". Trusted: " + str(prop.trusted)

	print "Blinking"
	for i in range(0,2):

		device.writeToCharacteristic(FIRMATA_CONFIG_UUID, 3, LED_ON)
		time.sleep(0.5)

		device.writeToCharacteristic(FIRMATA_CONFIG_UUID, 3, LED_OFF)
		time.sleep(0.5)

	# lb.registerCallbackEventRead(device, FIRMATA_DATA_UUID, firmata_callback_py)
	lb.registerCallbackChangeState(device, change_state_callback_py)
	
	print "get_version"
	get_version = lb.uintVector(3) 
	get_version[0] = 0xf0
	get_version[1] = 0x79
	get_version[2] = 0xf7

	device.writeToCharacteristic(FIRMATA_CONFIG_UUID, 3, get_version)

	print "waiting for callbacks"
	time.sleep(5)


	print "Disconnecting " +DEVICE_S
	device.disconnect()
	# device.unpair()
	print "waiting for callbacks"
	time.sleep(5)
	print DEVICE_S + " disconnected."
except Exception as e:
	print "EXCEPTION: " + str(e)
	if device:
		print "\tDisconnecting " +DEVICE_S


		device.disconnect()
		print "\t" + DEVICE_S + " disconnected."