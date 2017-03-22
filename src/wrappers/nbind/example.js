var nbind = require('nbind');

var lib = nbind.init().lib;


function ChangeStateCB(isolate, num){
	console.log("in JS state change callback.\nState changed: "+ num.toString());		
}


	
function firmata_callback(isolate, values)
{
        
	console.log("in firmata_callback. Array size: " + values.length);
	var s = "";
	for (var i = 0; i < values.length; i++) {
		s += values[i] + " ";
	}
	console.log(s);
}
function sensor_callback(isolate, values)
{
    console.log("in sensor_callback. Array size: " + values.length);
    console.log("raw data recived: ")
	var s = "";
	for (var i = 0; i < values.length; i++) {
		s += values[i] + " ";
	}
	console.log(s);

    console.log("\n");
    var rawTemp = (values[1] << 8) | values[0];
    console.log("\nIR temperature: " + (rawTemp / 128.0).toString());

    var rawAmbTemp = (values[3] << 8) | values[2];
    console.log("ambient temperature: "+ (rawAmbTemp / 128.0).toString());
    
    return 0;
}


var DEVICE = "FIRMATA";
// var DEVICE = "CC2650 SensorTag";
var firmata = true;
var IR_FIRMATA_DATA_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
var IR_FIRMATA_CONFIG_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
var IR_SENSOR_DATA_UUID = "f000aa01-0451-4000-b000-000000000000"
var IR_SENSOR_CONFIG_UUID = "f000aa02-0451-4000-b000-000000000000"
var LED_ON = [0x91, 0x20, 0x00];
var LED_OFF = [0x91,0x00 , 0x00];
var get_version = [0xf0, 0x79, 0xf7];
var flagOn = [0x01]

// initial call, or just call refresh directly


function main() {
	
	try {

		var dm = lib.DeviceManager.getInstance();
		console.log("Scanning...");
		dm.getBlDevices(10);
		console.log("Getting Device " + DEVICE)
		device = dm.getDeviceByName(DEVICE);

		device.connect();
		console.log("Connected to: " + device.getName() + "\nDevice MAC address: " + device.getAddress());
		var services = device.getBleDeviceServices();
		
		if (services !== null && device.getNumOfServices() <= 0){
			throw "No services found"; 
		} else{
			console.log("Services found")
			for (i = 0; i < device.getNumOfServices(); i++) {
				console.log("Service:")
				console.log(services[i].getPath() + "\t" + services[i].getUuid());
				characteristics = services[i].getCharacteristics();
				console.log("Num of Characteristics: " + characteristics.length);
				for (j =0; j < characteristics.length; j++) {
					console.log(characteristics[j].getPath() +"\t" + characteristics[j].getUuid());
				}
			}
		}

		// FIRMATA
		if (firmata) {
			console.log("Blinking");
			for (var i = 0; i < 10; i++){
				device.writeToCharacteristic(IR_FIRMATA_CONFIG_UUID, 3, LED_ON);
				device.writeToCharacteristic(IR_FIRMATA_CONFIG_UUID, 3, LED_OFF);
			}

			
			
			console.log("get_version");
			device.registerCallbackReadEvent(IR_FIRMATA_DATA_UUID, firmata_callback);
			// device.registerCallbackStateEvent(ChangeStateCB);
			device.writeToCharacteristic(IR_FIRMATA_CONFIG_UUID, 3, get_version);

			
		}else {

			console.log("Enable IR Temperature Sensor");
			device.writeToCharacteristic(IR_SENSOR_CONFIG_UUID, 1, flagOn);
			
			
			device.registerCallbackReadEvent(IR_SENSOR_DATA_UUID, sensor_callback);
			// device.registerCallbackStateEvent(ChangeStateCB);

		}
		setTimeout(function(){console.log("Existing")}, 10000);
			console.log("Disconnecting " + DEVICE);
			device.disconnect();
		
			
	}
	catch(err) {
		console.log("EXCEPTION: " + err);
		console.log("\tDisconnecting "+ DEVICE);
		device.disconnect();	
	}
}

main();