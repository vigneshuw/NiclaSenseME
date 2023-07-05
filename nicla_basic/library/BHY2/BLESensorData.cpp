#include <zephyr/logging/log.h>

#include "BLESensorData.h"


LOG_MODULE_REGISTER(BLE_SENSOR_DATA, CONFIG_SET_LOG_LEVEL);


static bool                         notify_sensor_enabled;

/** @brief  Indicate that the notification has been enabled.*/
static void ccc_sensor_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value) {
    notify_sensor_enabled = (value == BT_GATT_CCC_NOTIFY);
}


// Sensor Data Transport
BT_GATT_SERVICE_DEFINE(sensorDataTransport, 
    BT_GATT_PRIMARY_SERVICE(BT_UUID_SEN_DATA_SERV), 
        // Sensor Data Transfer Notify
        BT_GATT_CHARACTERISTIC(BT_UUID_SEN_DATA, 
                    BT_GATT_CHRC_NOTIFY, 
                    BT_GATT_PERM_NONE, NULL, NULL, 
                    NULL), 
        BT_GATT_CCC(ccc_sensor_cfg_changed, 
                    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);


int send_sensor_data_notify(uint8_t *data_buf) {
    // Check for notifications enabled
    if(!notify_sensor_enabled) {
        return -EACCES;
    }

    return bt_gatt_notify(NULL, &sensorDataTransport.attrs[2], data_buf, sizeof(data_buf));
}