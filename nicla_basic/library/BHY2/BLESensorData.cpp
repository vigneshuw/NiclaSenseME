#include <zephyr/logging/log.h>

#include "BLESensorData.h"


LOG_MODULE_REGISTER(BLE_SENSOR_DATA, CONFIG_SET_LOG_LEVEL);


static struct ns_sd_cb                 bt_cb_funcs;
static bool                         notify_sensor_enabled;


int sensor_data_callback_init(struct ns_sd_cb *callbacks) {
    if(callbacks){
        bt_cb_funcs.sensor_select_cb = callbacks->sensor_select_cb;
    } else {
        LOG_DBG("No callbacks for sensor select operation\n");
    }

    return 0;
}


static ssize_t sensor_select_ble_callback(struct bt_conn *conn, 
                    const struct bt_gatt_attr *attr, 
                    const void *buf, 
                    uint16_t len, uint16_t offset, uint8_t flags) {

    LOG_DBG("Sensor Select Operation, handle: %u, conn: %p", attr->handle, (void *)conn);

    if(offset != 0) {
        LOG_DBG("Sensor Select Write: Incorrect data offset\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }
    if(len > 1) {
        LOG_DBG("Sensor Select Write: Incorrect data length\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    if(bt_cb_funcs.sensor_select_cb) {
        // Call the appropriate function
        bt_cb_funcs.sensor_select_cb(((uint8_t *)buf)[0]);
    }
    return len;

};


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
        // Sensor Select
        BT_GATT_CHARACTERISTIC(BT_UUID_SEN_SEL, 
                    BT_GATT_CHRC_WRITE, 
                    BT_GATT_PERM_WRITE, NULL, sensor_select_ble_callback, NULL)
);


int sensor_send_data_notify(uint8_t *data_buf) {
    // Check for notifications enabled
    if(!notify_sensor_enabled) {
        return -EACCES;
    }

    return bt_gatt_notify(NULL, &sensorDataTransport.attrs[2], data_buf, sizeof(data_buf));
}