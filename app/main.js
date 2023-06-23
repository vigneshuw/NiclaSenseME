const {app, BrowserWindow, ipcMain, dialog} = require("electron")
const fs = require("fs")
const Buffer = require("buffer/").Buffer
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
let focusedServices = undefined
let focusedCharacteristics = undefined
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
        let serviceUUIDs = gattFirmware.service
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
        let fwCharacteristic = undefined
        let data = undefined
        let data_length = undefined
        let last_packet = 0
        let transfer_bytes = 200
        let main_buf = undefined
        let preamble_buf = Buffer.alloc(5)
        let concat_buf = undefined
        // Transfer
        let num_bytes_packets = undefined
        let rm_bytes = undefined
        // Progress bar
        let currentProgress = 0
        let totalProgress = undefined
        let crc = 0x00
        if(args.fw_type === "internalFWTransfer") {
            fwFile = internalFWPath
        } else {
            fwFile = externalFWPath
        }

        // Read the FW file
        if(fwFile) {
            data = fs.readFileSync(fwFile)

            // Prep for transfer
            data_length = data.length
            num_bytes_packets = Math.trunc(data_length / transfer_bytes)
            rm_bytes = data_length % transfer_bytes
            console.log(`Total number of bytes in firmware ${data_length}. 
            Number of packets = ${num_bytes_packets}, Remainder packets = ${rm_bytes}`)

            // Progress bar
            currentProgress = 1
            totalProgress = num_bytes_packets + ((rm_bytes) ? 1 : 0)

        } else {
            e.returnValue = false
        }

        // BLE params
        if((focusedServices.length === gattFirmware.service.length) &&
            (focusedCharacteristics.length === Object.values(gattFirmware.characteristics).length)) {

            // Identify the right characteristics
            if (args.fw_type === "internalFWTransfer") {
                focusedCharacteristics.forEach((characteristic, index) => {
                    if (characteristic.uuid === gattFirmware.characteristics["internal_fw"].split("-").join("")) {
                        fwCharacteristic = characteristic;
                    }
                })
            } else if (args.fw_type === "externalFWTransfer") {
                focusedCharacteristics.forEach((characteristic, index) => {
                    if (characteristic.uuid === gattFirmware.characteristics["external_fw"].split("-").join("")) {
                        fwCharacteristic = characteristic;
                    }
                })
            } else {
                e.returnValue = false
            }

        } else {
            e.returnValue = false
        }

        if(fwCharacteristic && data) {
            // Add a callback to characteristics write
            fwCharacteristic.on("write", () => {
                // Update progress bar
                mainWindow.webContents.send("FWProgress",
                    {"value": Math.trunc((currentProgress/totalProgress) * 100), "fw_type": args.fw_type})
                currentProgress ++;

            })

            // Control the write process
            let start_index = undefined
            let end_index = undefined
            for(let i = 0; i < num_bytes_packets; i++) {

                // Indexing bounds
                start_index = Math.trunc(i * transfer_bytes)
                end_index = Math.trunc(start_index + transfer_bytes)

                // When reaching the last byte
                if(i === (num_bytes_packets - 1)) {
                    if(rm_bytes === 0) {
                        // Set the last packet
                        last_packet = 1
                    }
                }

                // Configure the data appropriately
                main_buf = Buffer.from(data.slice(start_index, end_index))
                preamble_buf.writeUInt32LE(start_index, 1)
                preamble_buf.writeUInt8(last_packet, 0)
                concat_buf = Buffer.concat([preamble_buf, main_buf], transfer_bytes + 5)
                fwCharacteristic.write(concat_buf, false)

            }

            // Case of remainder bytes
            if(rm_bytes !== 0) {

                // Get the indexes
                start_index = end_index
                end_index = start_index + rm_bytes
                last_packet = 1

                main_buf = Buffer.from(data.slice(start_index, end_index))
                preamble_buf.writeUInt32LE(start_index, 1)
                preamble_buf.writeUInt8(last_packet, 0)
                concat_buf = Buffer.concat([preamble_buf, main_buf], rm_bytes + 5)
                fwCharacteristic.write(concat_buf, false)

            }

            // The Transfer is successfully complete
            e.returnValue = true;
        } else {
            e.returnValue = false;
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
