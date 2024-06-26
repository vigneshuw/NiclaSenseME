#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/pm/pm.h>
#include <zephyr/pm/device.h>
#include <zephyr/pm/policy.h>

#include "NiclaSystem.hpp"
#include "BHY2.h"
#include "BLESensorData.h"


LOG_MODULE_REGISTER(main, CONFIG_SET_LOG_LEVEL);


/*
Sensor Configuration
*/
#define STACK_SIZE              1024
#define PRIORITY                7
#define CHECK_INTERVAL          10
#define BLE_DATA_LEN            202


// Sensor Variables
SensorXYZ                       _accl_sensor(SENSOR_ID_ACC_PASS);
SensorXYZ                       _gyro_sensor(SENSOR_ID_GYRO_PASS);
SensorOrientation               _orin_sensor(SENSOR_ID_ORI);
SensorQuaternion                _rota_sensor(SENSOR_ID_RV);
SensorBSEC                      _bsec_sensor(SENSOR_ID_BSEC);

// Data update status
bool update_state = false;
uint8_t active_sensor = 0;
uint8_t data_preamble = 2;
uint8_t byte_counter = data_preamble;
uint8_t buf[BLE_DATA_LEN];

// Work Queue initialization
K_THREAD_STACK_DEFINE(sensor_work_q_stack_area, STACK_SIZE);
struct k_work_q sensor_work_q;
struct work_q_data {
    struct k_work work;
    uint8_t buf[BLE_DATA_LEN];
} sensor_work_q_data;
struct k_work_sync sensor_work_q_sync;

struct pm_state_info pms {PM_STATE_SOFT_OFF, 0, 0};


/*
BLE Setup for Sensor Data Transfer
*/
/** @brief  Set the active sensor */
void select_active_sensor(uint8_t sensor_id, uint16_t sampling_rate) {
    active_sensor = sensor_id;
    LOG_DBG("Sensor Selectiom: Received Sensor ID %u, with the sampling rate of %u", sensor_id, sampling_rate);

    switch (sensor_id)
    {
    case 0:
        // Disable all sensors
        _accl_sensor.begin(0, 0);
        _gyro_sensor.begin(0, 0);
        _orin_sensor.begin(0, 0);
        _rota_sensor.begin(0, 0);
        _bsec_sensor.begin(0, 0);

        // Reset some variables
        byte_counter = data_preamble;

        // Clear the work queue
        k_work_cancel(&sensor_work_q_data.work);

        // Clear the buffer
        sensortec.flushSensorData(SENSOR_ID_ACC_PASS);
        sensortec.flushSensorData(SENSOR_ID_GYRO_PASS);
        sensortec.flushSensorData(SENSOR_ID_ORI);
        sensortec.flushSensorData(SENSOR_ID_RV);
        sensortec.flushSensorData(SENSOR_ID_BSEC);

        // Go to deep sleep
        // pm_state_force(0u, &pms);
        break;

    case SENSOR_ID_ACC_PASS:
        _accl_sensor.begin(sampling_rate, 0);
        // 8g
        _accl_sensor.setRange(8);
        break;

    case SENSOR_ID_GYRO_PASS:
        _gyro_sensor.begin(sampling_rate, 0);
        // 250 degree/s
        _gyro_sensor.setRange(250);
        break;

    case SENSOR_ID_ORI:
        _orin_sensor.begin(sampling_rate, 0);
        break;

    case SENSOR_ID_RV:
        _rota_sensor.begin(sampling_rate, 0);
        break;

    case SENSOR_ID_BSEC:
        _bsec_sensor.begin(sampling_rate, 0);
        break;

    case 255:
        // All the IMU sensors
        _accl_sensor.begin(sampling_rate, 0);
        _gyro_sensor.begin(sampling_rate, 0);
        _orin_sensor.begin(sampling_rate, 0);
        break;
    
    default:
        break;
    }
}
// Set the callback struct
struct ns_sd_cb sensor_data_callbacks = {
    .sensor_select_cb = select_active_sensor
};
/** @brief  Sending the data over BLE */
void send_sensor_data(struct k_work *item) {
    // Get the pointer to the data structure
    struct work_q_data *data = 
        CONTAINER_OF(item, struct work_q_data, work);
    
    // Send over BLE
    sensor_send_data_notify(data->buf, sizeof(data->buf));
}


/** @brief  Process the data according to the active sensor. */
void process_sensor_data(void) {

    switch (active_sensor)
    {
    case 0:
        break;
    
    case SENSOR_ID_ACC_PASS:
    case SENSOR_ID_GYRO_PASS:
        {
            // Copy the data to buffer
            DataXYZ _sData;
            if(active_sensor == 1) {
                _sData = _accl_sensor.getData();
            } else {
                _sData = _gyro_sensor.getData();
            }

            bytecpy(&buf[byte_counter], &_sData, sizeof(_sData));
            byte_counter += sizeof(_sData);

            // Check if we need to send data
            if((byte_counter + sizeof(_sData) - data_preamble) > sizeof(buf) - data_preamble) {
                // Update length as first variable
                buf[0] = byte_counter - data_preamble;
                buf[1] = active_sensor;

                // Send data over BLE
                // Complete pending work
                bool work_state = k_work_flush(&sensor_work_q_data.work, &sensor_work_q_sync);

                //  Copy the new data 
                bytecpy(sensor_work_q_data.buf, buf, sizeof(buf));
                byte_counter = data_preamble;

                // Add the item to work queue
                k_work_submit_to_queue(&sensor_work_q, &sensor_work_q_data.work);
            }
        }
        break;

    case SENSOR_ID_ORI:
        {
            DataOrientation _sData;
            _sData = _orin_sensor.getData();

            bytecpy(&buf[byte_counter], &_sData, sizeof(_sData));
            byte_counter += sizeof(_sData);

            // Check if we need to send data
            if((byte_counter + sizeof(_sData) - data_preamble) > sizeof(buf) - data_preamble) {
                // Update length as first variable
                buf[0] = byte_counter - data_preamble;
                buf[1] = active_sensor;

                // Send data over BLE
                // Complete pending work
                bool work_state = k_work_flush(&sensor_work_q_data.work, &sensor_work_q_sync);

                //  Copy the new data 
                bytecpy(sensor_work_q_data.buf, buf, sizeof(buf));
                byte_counter = data_preamble;

                // Add the item to work queue
                k_work_submit_to_queue(&sensor_work_q, &sensor_work_q_data.work);
            }

        }
        break;

    case SENSOR_ID_RV:
        {
            DataQuaternion _sData;
            _sData = _rota_sensor.getData();

            bytecpy(&buf[byte_counter], &_sData, sizeof(_sData));
            byte_counter += sizeof(_sData);

            // Check if we need to send data
            if((byte_counter + sizeof(_sData) - data_preamble) > sizeof(buf) - data_preamble) {
                // Update length as first variable
                buf[0] = byte_counter - data_preamble;
                buf[1] = active_sensor;

                // Send data over BLE
                // Complete pending work
                bool work_state = k_work_flush(&sensor_work_q_data.work, &sensor_work_q_sync);

                //  Copy the new data 
                bytecpy(sensor_work_q_data.buf, buf, sizeof(buf));
                byte_counter = data_preamble;

                // Add the item to work queue
                k_work_submit_to_queue(&sensor_work_q, &sensor_work_q_data.work);
            }


        }
        break;

    case SENSOR_ID_BSEC:
        {
            DataBSEC _sData;
            _sData = _bsec_sensor.getData();

            bytecpy(&buf[byte_counter], &_sData, sizeof(_sData));
            byte_counter += sizeof(_sData);

            // Check if we need to send data
            if((byte_counter + sizeof(_sData) - data_preamble) > sizeof(buf) - data_preamble) {
                // Update length as first variable
                buf[0] = byte_counter - data_preamble;
                buf[1] = active_sensor;

                // Send data over BLE
                // Complete pending work
                bool work_state = k_work_flush(&sensor_work_q_data.work, &sensor_work_q_sync);

                //  Copy the new data 
                bytecpy(sensor_work_q_data.buf, buf, sizeof(buf));
                byte_counter = data_preamble;

                // Add the item to work queue
                k_work_submit_to_queue(&sensor_work_q, &sensor_work_q_data.work);
            }
        }
        break;

    case 255:
        {   
            // Get the data items
            DataXYZ _accData;
            DataXYZ _gyroData;
            DataOrientation _orinData;
            // Update the buffer
            _accData = _accl_sensor.getData();
            bytecpy(&buf[byte_counter], &_accData, sizeof(_accData));
            byte_counter += sizeof(_accData);
            _gyroData = _gyro_sensor.getData();
            bytecpy(&buf[byte_counter], &_gyroData, sizeof(_gyroData));
            byte_counter += sizeof(_gyroData);
            _orinData = _orin_sensor.getData();
            bytecpy(&buf[byte_counter], &_orinData, sizeof(_orinData));
            byte_counter += sizeof(_orinData);

            // Check if we need to send data
            uint16_t data_size = sizeof(_accData) + sizeof(_gyroData) + sizeof(_orinData);
            if((byte_counter + data_size - data_preamble) > sizeof(buf) - data_preamble) {
                // Update length as first variable
                buf[0] = byte_counter - data_preamble;
                buf[1] = active_sensor;

                // Send data over BLE
                // Complete pending work
                bool work_state = k_work_flush(&sensor_work_q_data.work, &sensor_work_q_sync);

                //  Copy the new data 
                bytecpy(sensor_work_q_data.buf, buf, sizeof(buf));
                byte_counter = data_preamble;

                // Add the item to work queue
                k_work_submit_to_queue(&sensor_work_q, &sensor_work_q_data.work);
            }
        }
        break;
    
    default:
        break;
    }

}


int main(void) {

    LOG_INF("Initializing the main loop\n");

    /*
    Initialize Nicla System
    */
    nicla::leds.begin();
    nicla::pmic.enableCharge(100);
    bool ret = nicla::pmic.enable3V3LDO();
    if(!ret) {
        LOG_ERR("3V3LDO failed!\n");
    }

    // Initialize BLE Callback struct
    sensor_data_callback_init(&sensor_data_callbacks);

    // Initialize BHI260
    bhy2.begin();
    
    // Work queue setup
    k_work_queue_init(&sensor_work_q);
    k_work_queue_start(&sensor_work_q, sensor_work_q_stack_area, 
                K_THREAD_STACK_SIZEOF(sensor_work_q_stack_area), PRIORITY, NULL);
    k_work_init(&sensor_work_q_data.work, send_sensor_data);

    while (1) {

        // Keep updating the sensor data
        update_state = bhy2.update(1);
        
        // Process the sensor data
        if(update_state) {
            process_sensor_data();
        }
        
    }

}
