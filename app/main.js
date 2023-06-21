const {app, BrowserWindow} = require("electron")
// The main window
let mainWindow

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 1200, height: 800,
        webPreferences: {nodeIntegration: true}
    })

    // Load some HTML into main Window
    mainWindow.loadFile("./index.html")

    // On closing the window
    mainWindow.on("closed", () => {

        // Disconnect the BLE and wrap-up

        mainWindow = null
    })
}

// When the app is ready
app.on('ready', createWindow)