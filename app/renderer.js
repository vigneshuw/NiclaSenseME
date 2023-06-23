const {ipcRenderer} = require("electron");

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


function FWLoad(id) {
    // load the file or get location
    let filePath = ipcRenderer.sendSync("S01_FWLoader", {fw_type: id})

    if(filePath) {
        if(id === "internalFWLoad") {
            // Enable the Check and Transfer Button
            document.getElementById("internalFWCheck").classList.remove("disabled")
            document.getElementById("internalFWTransfer").classList.remove("disabled")
        } else {
            // Enable the Check and Transfer Button
            document.getElementById("externalFWCheck").classList.remove("disabled")
            document.getElementById("externalFWTransfer").classList.remove("disabled")
        }

    }
}
function FWCheck(id) {
    let crc = ipcRenderer.sendSync("S01_FWCheck", {fw_type: id})
    if(id === "internalFWCheck") {
        document.getElementById("nRFCRCCheck").innerHTML = `
            <div class="alert alert-warning alert-dismissible fade show" role="alert">
                <strong>Firmware CRC</strong> is ${crc}.
                <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
            </div>
        `
    } else {
        document.getElementById("BHYCRCCheck").innerHTML = `
            <div class="alert alert-warning alert-dismissible fade show" role="alert">
                <strong>Firmware CRC</strong> is ${crc}.
                <button type="button" class="btn-close" data-bs-dismiss="alert" aria-label="Close"></button>
            </div>
        `
    }
}
function FWTransfer(id) {
    ipcRenderer.sendSync("S01_FWTransfer", {fw_type: id})

}