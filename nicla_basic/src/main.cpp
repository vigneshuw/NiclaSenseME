#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>

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
// Sensor Variables
SensorXYZ                       _accl_sensor(SENSOR_ID_ACC_PASS);
SensorXYZ                       _gyro_sensor(SENSOR_ID_GYRO_PASS);
SensorOrientation               _orin_sensor(SENSOR_ID_DEVICE_ORI);
SensorQuaternion                _rota_sensor(SENSOR_ID_RV);
// SensorBSEC                      _bsec_sensor(SENSOR_ID_BSEC);

// Data update status
bool update_state = false;
uint8_t active_sensor = 1;
uint8_t byte_counter = 0;
uint8_t buf[200];

// Work Queue initialization
K_THREAD_STACK_DEFINE(sensor_work_q_stack_area, STACK_SIZE);
struct k_work_q sensor_work_q;
struct work_q_data {
    struct k_work work;
    uint8_t buf[200];
} sensor_work_q_data;
struct k_work_sync sensor_work_q_sync;


/** @brief  Sending the data over BLE */
void send_sensor_data(struct k_work *item) {
    // Get the pointer to the data structure
    struct work_q_data *data = 
        CONTAINER_OF(item, struct work_q_data, work);
    
    // Send over BLE
    printk("Length of data received is %u\n", sizeof(data->buf));
}


/** @brief  Process the data according to the active sensor. */
void process_sensor_data(void) {

    switch (active_sensor)
    {
    case 1:
        {
            // Copy the data to buffer
            DataXYZ _sData = _accl_sensor.getData();
            bytecpy(&buf[byte_counter], &_sData, sizeof(_sData));
            byte_counter += sizeof(_sData);

            // Check if we need to send data
            if((byte_counter + sizeof(_sData)) > sizeof(buf) - 1) {
                // Send data over BLE
                // Complete pending work
                bool work_state = k_work_flush(&sensor_work_q_data.work, &sensor_work_q_sync);
                if (work_state) {
                    printk("Work pending!\n");
                }

                //  Copy the new data 
                bytecpy(sensor_work_q_data.buf, buf, sizeof(buf));
                byte_counter = 0;

                // Add the item to work queue
                k_work_submit(&sensor_work_q_data.work);
            }
        }
        break;

    // case 10:
    //     break;

    // case 69:
    //     break;

    // case 34:
    //     break;

    // case 115:
    //     break;
    
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

    // Initialize BHI260
    bhy2.begin();
    // Work queue setup
    k_work_queue_init(&sensor_work_q);
    k_work_queue_start(&sensor_work_q, sensor_work_q_stack_area, 
                K_THREAD_STACK_SIZEOF(sensor_work_q_stack_area), PRIORITY, NULL);
    k_work_init(&sensor_work_q_data.work, send_sensor_data);

    _accl_sensor.configure(50, 0);

    while (1) {

        // Keep updating the sensor data
        update_state = bhy2.update(CHECK_INTERVAL);
        
        // Process the sensor data
        if(update_state) {
            process_sensor_data();
        }
        
    }

}
