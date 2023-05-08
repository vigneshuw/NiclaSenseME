#include <zephyr/types.h>
#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include "NiclaService.hpp"


LOG_MODULE_DECLARE(NiclaBLE);


static bool firware_update_state;
static uint32_t boot_count;
static struct ns_cb bt_ns_callbacks;


/*
Callbacks
*/
/** @brief Callback for writing to the firware update characteristics*/
static ssize_t bhi_firmware_upload(struct bt_conn *conn, 
                    const struct bt_gatt_attr *attr, 
                    const void *buf, 
                    uint16_t len, uint16_t offset, uint8_t flags) {

    LOG_DBG("Attribute write, handle: %u, conn: %p", attr->handle, (void *)conn);

    if(len != 1U) {
        LOG_DBG("Write led: Incorrect data length\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    if (offset != 0) {
        LOG_DBG("Write len: Incorrect data offset\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    if (bt_ns_callbacks.firmware_update_cb) {

        // Read the received value
        uint8_t val = *((uint8_t *) buf);

        if(val == 0x00 || val == 0x01) {
            bt_ns_callbacks.firmware_update_cb(val ? true : false);
        } else {
            LOG_DBG("Write led: Incorrect value");
            return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
        }

    }

    return len;

}

static ssize_t bhi_firmware_data(struct bt_conn *conn, 
                    const struct bt_gatt_attr *attr,
                    const void *buf,
                    uint16_t len, uint16_t offset, uint8_t flags) {
    
    LOG_DBG("Attribute write, handle: %u, conn: %p", attr->handle, (void *)conn);

    if (offset != 0) {
        LOG_DBG("Write FW data: Incorrect data offset\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    if (bt_ns_callbacks.firmware_data_cb) {

        // Read the received value
        uint8_t *val = (uint8_t *)buf;
        bt_ns_callbacks.firmware_data_cb(val, len);

    }

    return len;

}


/** @brief Callback for reading the boot count in GATT Characteristics*/
static ssize_t read_boot_count(struct bt_conn *conn,
                    const struct bt_gatt_attr *attr,
                    void *buf, 
                    uint16_t len,
                    uint16_t offset) {

    // Get a pointer to user data value specified in characteristics initialization                    
    const char *value = (char*) attr->user_data;

    LOG_DBG("Attribute read, handle: %u, conn: %p", attr->handle,
		(void *)conn);

    if(bt_ns_callbacks.boot_cnt_cb) {
        // Update the current boot count
        boot_count = bt_ns_callbacks.boot_cnt_cb();
        return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(*value));
    }

    return 0;

}


/** @brief A function to register the callbacks*/
int nicla_service_init(struct ns_cb *callbacks) {
    if(callbacks) {
        bt_ns_callbacks.boot_cnt_cb = callbacks->boot_cnt_cb;
        bt_ns_callbacks.firmware_update_cb = callbacks->firmware_update_cb;
        bt_ns_callbacks.firmware_data_cb = callbacks->firmware_data_cb;
    }

    return 0;

}


/*
BLE Service and Characteristics 
*/
// BLE Service
BT_GATT_SERVICE_DEFINE(NiclaService, 
BT_GATT_PRIMARY_SERVICE(BT_UUID_NS),
    // Read of Boot Count
    BT_GATT_CHARACTERISTIC(BT_UUID_NS_BTC, 
                    BT_GATT_CHRC_READ, 
                    BT_GATT_PERM_READ, read_boot_count, NULL, 
                    &boot_count),
    // Control BHI260AP Firmware Update
    BT_GATT_CHARACTERISTIC(BT_UUID_NS_BHI_FU, 
                    BT_GATT_CHRC_WRITE, 
                    BT_GATT_PERM_WRITE, NULL, bhi_firmware_upload, NULL),
    // Firware data upload
    BT_GATT_CHARACTERISTIC(BT_UUID_NS_BHI_FW_DAT, 
                    BT_GATT_CHRC_WRITE, 
                    BT_GATT_PERM_WRITE, NULL, bhi_firmware_data, NULL)
);

