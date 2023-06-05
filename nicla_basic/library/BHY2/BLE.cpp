#include <zephyr/types.h>
#include <zephyr/logging/log.h>

#include "BLE.h"


LOG_MODULE_REGISTER(MD_BLE, CONFIG_SET_LOG_LEVEL);

static struct ns_cb bt_cb_funcs;

int callback_init(struct ns_cb *callbacks) {
    if(callbacks) {
        bt_cb_funcs.firmware_data_cb = callbacks->firmware_data_cb;
        bt_cb_funcs.sensor_config_cb = callbacks->sensor_config_cb;
        bt_cb_funcs.firmware_update_cb = callbacks->firmware_update_cb;
        LOG_DBG("BLE Callbacks are initialized\n");
    } else {
        LOG_DBG("No callbacks for BLE events\n");
    }

    return 0;
}


/** @brief The main BLE callback function for internal DFU*/
static ssize_t dfu_internal_ble_callback(struct bt_conn *conn, 
                    const struct bt_gatt_attr *attr,
                    const void *buf,
                    uint16_t len, uint16_t offset, uint8_t flags) {
    
    LOG_DBG("DFU Internal write, handle: %u, conn: %p", attr->handle, (void *)conn);

    if (offset != 0) {
        LOG_DBG("DFU Internal write: Incorrect data offset\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    if (bt_cb_funcs.firmware_data_cb) {
        // Call the Firmware save function
        bt_cb_funcs.firmware_data_cb(DFU_INTERNAL, buf, len);
    }

    return len;
};


/** @brief The main BLE callback function for external DFU*/
static ssize_t dfu_external_ble_callback(struct bt_conn *conn,
                    const struct bt_gatt_attr *attr,
                    const void *buf,
                    uint16_t len, uint16_t offset, uint8_t flags) {
    
    LOG_DBG("DFU External write, handle: %u, conn: %p", attr->handle, (void *)conn);

    if(offset != 0) {
        LOG_DBG("DFU External write: Incorrect data offset\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    if (bt_cb_funcs.firmware_data_cb) {
        // Call the Firmware save function
        bt_cb_funcs.firmware_data_cb(DFU_EXTERNAL, buf, len);
    }

    return len;
}


/** @brief The main BLE callback function that starts the FW update process */
static ssize_t dfu_firmware_update_enable(struct bt_conn *conn,
                    const struct bt_gatt_attr *attr,
                    const void *buf,
                    uint16_t len, uint16_t offset, uint8_t flags) {
    
    LOG_DBG("DFU Firmware update init, handle: %u, conn: %p", attr->handle, (void *)conn);

    if(offset != 0) {
        LOG_DBG("DFU External write: Incorrect data offset\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    } 

    if(bt_cb_funcs.firmware_update_cb) {
        // Start the firmware update process
        uint8_t val = ((uint8_t *)buf)[0];
        if(val == 0x00) {
            // BHY2 Firmware update
            bt_cb_funcs.firmware_update_cb(DFU_EXTERNAL);
        } else if (val == 0xFF) {
            // nRF52 Firmware update
            bt_cb_funcs.firmware_update_cb(DFU_INTERNAL);
        } else {
            LOG_WRN("Invalid FW update specifier\n");
        }
    }

    return len;
}


/** @brief The main BLE callback function for Sensor Config Update */
static ssize_t sensor_config_ble_callback(struct bt_conn *conn, 
                    const struct bt_gatt_attr *attr, 
                    const void *buf,
                    uint16_t len, uint16_t offset, uint8_t flags) {

    LOG_DBG("Sensor Config write, handle: %u, conn: %p", attr->handle, (void *)conn);

    if(offset != 0) {
        LOG_DBG("Sensor Config Write: Incorrect data offset\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    if(bt_cb_funcs.sensor_config_cb) {
        // Call the Sensor Config Update function
        bt_cb_funcs.sensor_config_cb(buf, len);
    }

    return len;

}


/*
BLE Service and Characteristics
*/
// Firmware Service
BT_GATT_SERVICE_DEFINE(DeviceFirmwareService,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_DFU_SERV), 
        // Internal Firmware
        BT_GATT_CHARACTERISTIC(BT_UUID_DFU_INTERNAL,
                        BT_GATT_CHRC_WRITE,
                        BT_GATT_PERM_WRITE, NULL, dfu_internal_ble_callback, NULL),
        // External Firmware
        BT_GATT_CHARACTERISTIC(BT_UUID_DFU_EXTERNAL, 
                        BT_GATT_CHRC_WRITE, 
                        BT_GATT_PERM_WRITE, NULL, dfu_external_ble_callback, NULL),
        // Enable the FW Update
        BT_GATT_CHARACTERISTIC(BT_UUID_DFU_ENABLE, 
                        BT_GATT_CHRC_WRITE,
                        BT_GATT_PERM_WRITE, NULL, dfu_firmware_update_enable, NULL)
);
// Sensor Config Service
BT_GATT_SERVICE_DEFINE(SensorConfigService,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_SEN_SERV),
        // Sensor Configuration
        BT_GATT_CHARACTERISTIC(BT_UUID_SEN_CONFIG, 
                    BT_GATT_CHRC_WRITE, 
                    BT_GATT_PERM_WRITE, NULL, sensor_config_ble_callback, NULL)
);
