#include "BHY2.h"

#include <zephyr/logging/log.h>
#include "BoschSensortec.h"
#include "BoschParser.h"
#include "NiclaSystem.hpp"
#include "BLEHandler.h"


LOG_MODULE_REGISTER(BHY2_SYSTEM, CONFIG_SET_LOG_LEVEL);


BHY2::BHY2() : 
    _pingTime(0),
    _timeout(120000),
    _startTime(0)
{
}

BHY2::~BHY2() {}

void BHY2::setLDOTimeout(int time) {
    _timeout = time;
}

bool BHY2::begin() {
    bool res;

    // Initialize Nicla System
    _startTime = k_uptime_get();
    res = nicla::pmic.enable3V3LDO();
    _pingTime = k_uptime_get();

    // Initialize the Sensor System
    res = sensortec.begin() & res;

    // Init BLE System
    res = bleHandler.begin() & res;

    // Init DFUManager
    res = dfuManager.begin() & res; 

    return res;

}

void BHY2::update() {
    sensortec.update();

    // FW transfer status 
    if (dfuManager.isPending()) {
        LOG_DBG("Stopping the execution of thread until FW Update is complete\n");
        while(dfuManager.isPending()) {
            k_msleep(1000);
        }

        LOG_DBG("Firmware transfer complete.");
    }
}

void BHY2::update(unsigned long ms) {
    update();
    k_msleep(ms);
}

void BHY2::delay(unsigned long ms) {
    k_msleep(ms);
}

void BHY2::configureSensor(SensorConfigurationPacket &config) {
    sensortec.configureSensor(config);
}

void BHY2::configureSensor(uint8_t sensorID, float sampleRate, uint32_t latency) {
    SensorConfigurationPacket config;
    config.sensorId = sensorID;
    config.sampleRate = sampleRate;
    config.latency = latency;

    sensortec.configureSensor(config);
}

void BHY2::addSensorData(SensorDataPacket &sensorData) {
    sensortec.addSensorData(&sensorData);
}

void BHY2::addLongSensorData(SensorLongDataPacket &sensorData) {
    sensortec.addLongSensorData(&sensorData);
}

uint8_t BHY2::availableSensorData() {
    return sensortec.availableSensorData();
} 

uint8_t BHY2::availableLongSensorData() {
    return sensortec.availableLongSensorData();
}

bool BHY2::readSensorData(SensorDataPacket *data) {
    return sensortec.readSensorData(data);
}

bool BHY2::readLongSensorData(SensorLongDataPacket *data) {
    return sensortec.readLongSensorData(data);
}

bool BHY2::hasSensor(uint8_t sensorID) {
    return sensortec.hasSensor(sensorID);
}

void BHY2::parse(SensorDataPacket &data, DataXYZ &vector) {
    DataParser::parse3Dvector(data, vector);
}

void BHY2::parse(SensorDataPacket &data, DataOrientation &vector) {
    DataParser::parseEuler(data, vector);
}

void BHY2::parse(SensorDataPacket &data, DataOrientation &vector, float scaleFactor) {
    DataParser::parseEuler(data, vector, scaleFactor);
}

BHY2 bhy2;
