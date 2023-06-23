const {app, BrowserWindow, ipcMain, dialog} = require("electron")
const fs = require("fs")
// BLE
const noble = require("@abandonware/noble")
const {gattFirmware} = require("./ble")
// The main window
let mainWindow

/*
BLE Connection Parameters
 */
// BLE device connection Indicator
let is_device_connected = false
let bleDevice = undefined
let internalFWPath = undefined
let externalFWPath = undefined
// Callbacks for Connection
function onPeripheralConnectCallback() {
    is_device_connected = true;
    noble.stopScanningAsync().catch((e) => {
        console.log(`Scanning cannot be stopped - ${e}`)
    })
    console.log("Connected to device")

    // Load the sensor access page
    mainWindow.loadFile("./pages/op_selector.html");

}
function onPeripheralDisconnectCallback() {
    is_device_connected = false;
    noble.startScanningAsync().catch((e) => {
        console.log(`Scanning failed due to - ${e}`);
    })
    console.log("Disconnected from device")

    // Go back to the index page
    mainWindow.loadFile("./index.html");

    // Discard the BLE Device
    bleDevice = undefined;
    focusedServices = undefined;
    focusedCharacteristics = undefined;
    internalFWPath = undefined
    externalFWPath = undefined

}
let focusedServices = undefined
let focusedCharacteristics = undefined

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 1200, height: 800,
        webPreferences: {nodeIntegration: true, contextIsolation: false},
        resizable: false,
        center: true,
        show: false,
        maximizable: false
    })

    // Load some HTML into main Window
    mainWindow.loadFile("./index.html").catch((e) => {
        console.log(`Unable to hold file - ${e}`)
    })
    mainWindow.once('ready-to-show', mainWindow.show)

    // On closing the window
    mainWindow.on("closed", () => {
        mainWindow = null
    })

    /*
    IPC Renderer Communication
     */
    // Choice on type of Sentinel and Connection
    ipcMain.on("Sentinel01", (e, args) => {
        /*
        BLE Connection
         */
        // Initiate BLE Connection to device
        noble.startScanningAsync().catch((e) => {
            console.log(`Scanning failed due to - ${e}`);
        })
        // On device discover - connect
        noble.on("discover", (peripheral) => {
            if(peripheral.advertisement.localName) {
                if(peripheral.advertisement.localName === "Sentinel01") {
                    console.log("Found device " + peripheral.advertisement.localName)
                    peripheral.on("connect", onPeripheralConnectCallback)
                    peripheral.on("disconnect", onPeripheralDisconnectCallback)
                    // Connect to device
                    peripheral.connectAsync().then(() => {
                        // Manage the global device
                        bleDevice = peripheral;

                    }).catch((e) => {
                        console.log(`Cannot connect to device ${peripheral.advertisement.localName}, Error - ${e}`)
                    })
                }
            }
        });
    });

    /*
    FW Services
     */
    // FW Services Connection
    ipcMain.on("S01_FWUpdate", (e, args) => {
        let serviceUUIDs = [gattFirmware.service]
        let characteristicsUUIDs = Object.values(gattFirmware.characteristics)
        bleDevice.discoverSomeServicesAndCharacteristics(serviceUUIDs, characteristicsUUIDs, (error, service, characteristics) => {
            // Bring the items to focus
            focusedServices = service;
            focusedCharacteristics = characteristics;
            // Go to the FW Page
            mainWindow.loadFile("./pages/fw.html").catch((e) => {
                console.log(`Cannot load the Firmware Page - ${e}`);
            })
        });
    })
    // FW Loader
    ipcMain.on("S01_FWLoader", (e, args) => {

        // Identify Firmware
        let fw_type;
        if (args.fw_type === "internalFWLoad") {
            fw_type = "Internal Firmware"
        } else {
            fw_type = "External Firmware"
        }

        // Get the filepath
        let filePath = dialog.showOpenDialogSync(mainWindow, {
            title: "Select Firmware File",
            properties: ["openFile"],
            buttonLabel: "Select",
        })
        if (fw_type === "Internal Firmware") {
            internalFWPath = filePath[0]
        } else {
            externalFWPath = filePath[0]
        }
        e.returnValue = filePath[0]

    })
    ipcMain.on("S01_FWCheck", (e, args) => {

        // Compute the CRC
        let fwFile = undefined
        let crc = 0x00
        if(args.fw_type === "internalFWCheck") {
            fwFile = internalFWPath
        } else {
            fwFile = externalFWPath
        }

        // Read the firmware data and compute CRC
        console.log(fwFile)
        if(fwFile) {
            let data = fs.readFileSync(fwFile)
            console.log(`The firmware loaded has a size of ${data.length}`)

            for (let i = 0; i < data.length; i++) {
                crc = crc ^ data[i]
            }
            console.log(`The CRC is ${crc}`)
            e.returnValue = crc
        }
    })
    ipcMain.on("S01_FWTransfer", (e, args) => {

        // Data params
        let fwFile = undefined
        let crc = 0x00
        if(args.fw_type === "internalFWCheck") {
            fwFile = internalFWPath
        } else {
            fwFile = externalFWPath
        }
        // BLE params
        

        // Read data and transfer
        console.log(fwFile)
        if(fwFile) {
            let data = fs.readFileSync(fwFile)
            console.log(`The firmware data for transfer has a size of ${data.length}`)


        }
    })



}

// When the app is ready
app.on('ready', createWindow)

// Quitting the app
app.on('before-quit', () => {
    // Disconnect the BLE and wrap-up
})


// BLEConfiguration
let device_found = false;

noble.startScanningAsync().catch((e) => {
    console.log(`Scanning stopped due to - ${e}`);
})
