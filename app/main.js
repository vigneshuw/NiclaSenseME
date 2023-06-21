const {app, BrowserWindow, dialog} = require("electron")
// The main window
let mainWindow

// BLE device connections
let is_device_connected = false

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 1200, height: 800,
        webPreferences: {nodeIntegration: true},
        resizable: false,
        center: true,
        show: false,
        maximizable: false
    })

    // Load some HTML into main Window
    mainWindow.loadFile("./index.html")
        .then(() => {
            mainWindow.once('ready-to-show', mainWindow.show)
        })


    // On closing the window
    mainWindow.on("closed", () => {
        mainWindow = null
    })
}

// When the app is ready
app.on('ready', createWindow)

// Quitting the app
app.on('before-quit', () => {
    // Disconnect the BLE and wrap-up

})