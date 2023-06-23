const {ipcRenderer} = require("electron");

/*
Update the progress bar in real-time as the firmware is transferred
 */
ipcRenderer.on("FWProgress", (e, message) => {
    if(message.fw_type === "internalFWTransfer") {
        document.getElementById("internalFWProgress").style.width = `${message.value}%`;
        document.getElementById("internalFWProgress").innerHTML = `${message.value}%`

        if(message.value === 100) {
            document.getElementById("nRFTransferCheck").innerHTML = `
            <div class="alert alert-success alert-dismissible fade show" role="alert">
                <strong>Firmware Write</strong> complete.
                <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
            </div>
            `
            document.getElementById("IntFWST").classList.remove("disabled")
            document.getElementById("IntFWSP").classList.remove("disabled")
        }

    } else {
        document.getElementById("externalFWProgress").style.width = `${message.value}%`;
        document.getElementById("externalFWProgress").innerHTML = `${message.value}%`

        if(message.value === 100) {
            document.getElementById("BHYTransferCheck").innerHTML = `
            <div class="alert alert-success alert-dismissible fade show" role="alert">
                <strong>Firmware Write</strong> complete.
                <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
            </div>
            `
            document.getElementById("ExtFWST").classList.remove("disabled")
            document.getElementById("ExtFWSP").classList.remove("disabled")
        }
    }
})


/**
 * @brief   Initiate a connection to a specific Sentinel
 *
 */
function deviceSentinel01INIT(){

    // Disable all buttons
    const cells = document.getElementsByTagName("button")
    for (const cell of cells) {
        cell.classList.add("disabled")
    }
    // Indicate connection state
    document.getElementById("Sentinel01Init").innerHTML = "Connecting..."

    // Sentinel Selection
    ipcRenderer.send("Sentinel01");
}


/*
FW Update for the device
 */
/**
 * @brief   Get the right services and characteristics for the operation
 *          This is for the Firmware Transfer
 */
function fwServiceInit() {

    // Disable all buttons
    const cells = document.getElementsByTagName("button")
    for (const cell of cells) {
        cell.classList.add("disabled")
    }
    // Indicate connection state
    document.getElementById("fwInitButton").innerHTML = "Initializing Services..."

    // Connect to Main Process
    ipcRenderer.send("S01_FWUpdate");

}
/**
 * @brief Load the firmware file, i.e., get the file path for the firmware
 *
 * @param id The ID corresponding to the button that was pressed
 *
 */
function FWLoad(id) {
    // load the file or get location
    let filePath = ipcRenderer.sendSync("S01_FWLoader", {fw_type: id})

    if(filePath) {
        if(id === "internalFWLoad") {
            // Enable the Check and Transfer Button
            document.getElementById("internalFWCheck").classList.remove("disabled")
        } else {
            // Enable the Check and Transfer Button
            document.getElementById("externalFWCheck").classList.remove("disabled")
        }

    }
}

/**
 * @brief   Check the CRC of the loaded firmware and display the current CRC at a specific location depending on the type
 * of firmware loaded (internal or external)
 *
 * @param id The ID corresponding to the button that was pressed
 * @constructor
 */
function FWCheck(id) {
    let crc = ipcRenderer.sendSync("S01_FWCheck", {fw_type: id})
    if(id === "internalFWCheck") {
        document.getElementById("nRFCRCCheck").innerHTML = `
            <div class="alert alert-warning alert-dismissible fade show" role="alert">
                <strong>Firmware CRC</strong> is ${crc}.
                <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
            </div>
        `
        document.getElementById("internalFWTransfer").classList.remove("disabled")
    } else {
        document.getElementById("BHYCRCCheck").innerHTML = `
            <div class="alert alert-warning alert-dismissible fade show" role="alert">
                <strong>Firmware CRC</strong> is ${crc}.
                <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
            </div>
        `
        document.getElementById("externalFWTransfer").classList.remove("disabled")
    }
}

/**
 * @brief   Initiate the transfer of the firmware to the device over BLE
 *
 * @param id    The ID corresponding to the button that was pressed
 * @constructor
 */
function FWTransfer(id) {
    let writeStatus = ipcRenderer.sendSync("S01_FWTransfer", {fw_type: id})
    if(writeStatus) {

        if(id === "internalFWTransfer") {
            document.getElementById("nRFTransferCheck").innerHTML = `
            <div class="alert alert-warning alert-dismissible fade show" role="alert">
                <strong>Firmware Write</strong> in progress.
                <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
            </div>
            `
        } else if (id === "externalFWTransfer") {
            document.getElementById("BHYTransferCheck").innerHTML = `
            <div class="alert alert-warning alert-dismissible fade show" role="alert">
                <strong>Firmware Write</strong> in progress.
                <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
            </div>
            `
        }
    }
}