#include "BLEHandler.h"
#include "sensors/SensorID.h"
#include "BoschSensortec.h"
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(MD_BLEHandler, CONFIG_SET_LOG_LEVEL);

/*
BLE Connection Advertising
*/
// Advertising params
static struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM((BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_IDENTITY), 
    800, 801, NULL);
// Advertising data
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};
// Scanning response data
static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123))
};

/*
Initialize Static variables
*/
bool BLEHandler::_lastDFUPack = false;
bool BLEHandler::bleActive = false;
struct ns_cb BLEHandler::app_callbacks = {
    .firmware_data_cb = processDFUPacket,
    .sensor_config_cb = processSensorConfig,
    .firmware_update_cb = writeDFUFirmwareToFlash
};
struct bt_conn_cb BLEHandler::conn_callbacks = {
    .connected = on_connected,
    .disconnected = on_disconnected
};



BLEHandler::BLEHandler() {

}

BLEHandler::~BLEHandler()
{

}

bool BLEHandler::begin() {

    // Prevent multiple calls to begin
    if(bleActive) {
        LOG_DBG("BLE is already active! End it to begin\n");
        return false;
    }

    /*
    BLE Connection
    */
    // Registering the callbacks
    int err;
    bt_conn_cb_register(&conn_callbacks);
    err = callback_init(&app_callbacks);
    if(err) {
        LOG_ERR("The Read-Write callbacks cannot be initialized\n");
        return false;
    }

    // Enable BLE
    err = bt_enable(NULL);
    if(err) {
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return false;
    }
    LOG_INF("BLE initialized; Callbacks sucessfully set\n");
    // Start Advertising
    err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if(err) {
        LOG_ERR("Advertising failed to start (err %d)\n", err);
        return false;
    }
    LOG_INF("Advertising....\n");

    // Set BLE to be active
    bleActive = true;
    return true;
}

bool BLEHandler::end() {
    // Stop advertising and BLE
    int err;
    err = bt_le_adv_stop();
    if(err) {
        LOG_ERR("Cannot stop advertising (err %d)\n", err);
        return false;
    }
    err = bt_disable();
    if(err) {
        LOG_ERR("Cannot stop BLE (err %d)\n", err);
        return false;
    }

    bleActive = false;
    return true;
}

void BLEHandler::update() {
    LOG_WRN("NOT IMPLEMENTED\n");
}

void BLEHandler::processDFUPacket(DFUType dfuType, const void *buf, uint16_t len) {
    
    LOG_DBG("Size of data received: %u\n", len);

    // Process packet through DFU Manager
    const uint8_t *val = (const uint8_t *)buf;
    dfuManager.processPacket(dfuType, val, len);

    // Check for the last packet
    if(val[0]) {
        _lastDFUPack = true;
        dfuManager.closeDfu();
    }

}

void BLEHandler::writeDFUFirmwareToFlash(DFUType dfuType) {
    LOG_DBG("Firmware write has been initialized\n");

    int rc = dfuManager.writeFirmwareToFlash(dfuType);
    if(rc) {
        LOG_ERR("The Firmware write has failed\n");
    } else {
        LOG_DBG("Firmware has been successfully written!\n");
    }
}

void BLEHandler::on_connected(struct bt_conn *conn, uint8_t err) {
    if(err) {
        LOG_ERR("Connection error %u\n", err);
    }
    LOG_DBG("Connection established\n");
}

void BLEHandler::on_disconnected(struct bt_conn *conn, uint8_t reason) {
    LOG_DBG("Disconnected. Reason %u\n", reason);
}

void BLEHandler::processSensorConfig(const void *buf, uint16_t len) {
    // Get data
    SensorConfigurationPacket data;
    bytecpy(&data, buf, len);
    
    // LOG Received Configuration
    LOG_DBG("Configuration Received: \n");
    LOG_DBG("\tSensor ID: %u; Sampling Rate: %f; Latency: %u\n", data.sensorId, data.sampleRate, data.latency);

    // Write the sensor configuration
    sensortec.configureSensor(data);
}

BLEHandler bleHandler;