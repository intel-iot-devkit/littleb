/**
 * Created by ryehudai on 2/27/2017.
 */

var async = require('asyncawait/async');
var await = require('asyncawait/await');
var Promise = require('bluebird');
var nbind = require('nbind');
var lib = nbind.init().lib;
//var dm = lib.DeviceManager.getInstance();


function getPromise(sFunc) {
    var args = Array.prototype.slice.call(arguments, 1);

    return new Promise(function (resolve, reject) {
        try {
            // console.log('STARTED promise');
            var res = sFunc(args);
            resolve (res);
            // console.log('ENDED promise');
        }
        catch (err) {
            reject(err);
        }
    });

}


function LBDeviceManager() {
   
    this.scanForBleDevices = async(function(secs) {
        return await(getPromise(function (args) {
            console.log('Scanning for BLE devices for '+args[0]+' seconds...');
            lib.DeviceManager.getInstance().getBlDevices(args[0]);
        }, secs));
    });
    this.getDevice = async(function(deviceName) {
        return await(getPromise(function (args) {
            console.log('Getting device: '+args[0]);
            return new JSDevice(lib.DeviceManager.getInstance().getDeviceByName(args[0]));
        }, deviceName));
    });

   
}

function JSDevice(nativeDev)
{
    var dev = nativeDev;

    this.connect =  async(function () {
        return await(getPromise(function() {
            console.log("Connecting to device...");
            dev.connect();
        }))});

    this.disconnect = async(function () {
        return await(getPromise(function() {
            console.log("Disconnecting...");
            dev.disconnect();
        }))});

    this.pair =  async(function () {
        return await(getPromise(function() {
            console.log("Pairing Device...");
            dev.pair();
        }))});

    this.unpair = async(function () {
        return await(getPromise(function() {
            console.log("UnPairing Device...");
            dev.unpair();
        }))});

    this.getBleCharacteristicByPath = async(function (path) {
        return await(getPromise(function() {
            console.log("Getting BLE Charac' by path "+ args[0]);
            dev.getBleCharacteristicByPath(args[0]);
        }, path))});

    this.getBleCharacteristicByUuid = async(function (uuid) {
        return await(getPromise(function() {
            console.log("Getting BLE Charac' by UUID "+ args[0]);
            dev.getBleCharacteristicByUuid(args[0]);
        }, uuid))});

    this.getBleServiceByPath = async(function (path) {
        return await(getPromise(function() {
            console.log("Getting BLE Service by path "+ args[0]);
            dev.getBleServiceByPath(args[0]);
        }, path ))});

    this.getBleServiceByUuid =  async(function (uuid) {
        console.log("Getting BLE Service by UUID "+ args[0]);
        return await(getPromise(function() {
            dev.getBleServiceByUuid(args[0]);
        }, uuid))});

    this.getBleDeviceServices = async(function () {
        return await(getPromise(function(args) {
            console.log("Getting BLE Device Services from device");
            return dev.getBleDeviceServices();
        }))});

    this.writeToCharacteristic = async(function (uuid, size, value){
        return await(getPromise(function(args) {
            console.log("Writing to Charac' UUID "+ args[0] + " size "+ args[1] + " value " + args[2]);
            return dev.writeToCharacteristic(args[0], args[1], args[2]);
        }, uuid, size, value))});

    this.readFromCharacteristic= async(function (uuid) {
        return await(getPromise(function(args) {
            console.log("Reading from Charac' by UUID "+ args[0]);
            return dev.readFromCharacteristic(args[0]);
        }, uuid))});
    
    //properties
    this.getDeviceProperties = async(function () {
        return await(getPromise(function() {
            return dev.getDeviceProperties()
        }))});

    this.getName = async(function () {
        return await(getPromise(function() {
            return dev.getName()
        }))});

    this.getAddress = async(function () {
        return await(getPromise(function(){
            return dev.getAddress()
        }))});

    this.getNumOfServices = async(function () {
        return await(getPromise(function() {
            return dev.getNumOfServices()
        }))});

    //register callbacks
    this.registerCallbackStateEvent = function(func) {dev.registerCallbackStateEvent(func);};
    this.registerCallbackReadEvent = function(uuid, func) {dev.registerCallbackReadEvent(uuid, func);};

}

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
var IR_FIRMATA_DATA_UUID = "6e400003-b5a3-f393-e0a9-e50e24dcca9e"
var IR_FIRMATA_CONFIG_UUID = "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
var LED_ON = [0x91, 0x20, 0x00];
var LED_OFF = [0x91,0x00 , 0x00];
var get_version = [0xf0, 0x79, 0xf7];
// Another synchronous-looking function written in async/await style.
var program = async (function () {
    var device = null;
    try  {
        console.log('Firmata sample started...');
        
        var devMan = new LBDeviceManager();
        await(devMan.scanForBleDevices(7));       
        var device = await(devMan.getDevice("FIRMATA"));
        await(device.connect());
        
        console.log("=NAME: "+await(device.getName()));
        console.log("=ADDRESS: "+await(device.getAddress()));
        var services = await(device.getBleDeviceServices());       
        console.log("=NUM OF SERVICES: "+ await(device.getNumOfServices()));

        if (services !== null && await(device.getNumOfServices()) <= 0){
            throw "No services found"; 
        } else{
            console.log("Services found")
            for (i = 0; i < await(device.getNumOfServices()); i++) {
                console.log("=Service:")
                console.log(services[i].getPath() + "\t" + services[i].getUuid());
                characteristics = services[i].getCharacteristics();
                console.log("=Num of Characteristics: " + characteristics.length);
                for (j =0; j < characteristics.length; j++) {
                    console.log(characteristics[j].getPath() +"\t" + characteristics[j].getUuid());
                }
            }
        }
        console.log("Blinking");
        for (var i = 0; i < 10; i++){
            await(device.writeToCharacteristic(IR_FIRMATA_CONFIG_UUID, 3, LED_ON));    
            await(device.writeToCharacteristic(IR_FIRMATA_CONFIG_UUID, 3, LED_OFF));
        }
        
        await(device.registerCallbackReadEvent(IR_FIRMATA_DATA_UUID, firmata_callback));
        await(device.registerCallbackStateEvent(ChangeStateCB));
        console.log("get_version");
        await(device.writeToCharacteristic(IR_FIRMATA_CONFIG_UUID, 3, get_version));
        setTimeout(function(){console.log("Sample exited successfully.")}, 10000);     
    } catch (ex) {
        console.log('Caught an error ' + ex);
        
    }
    if (device){
        await(device.disconnect());

    }
    return 'Finished!';
});

// Execute program() and print the result.
program().then(function (result) {
    console.log(result);
});

