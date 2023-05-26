#ifndef _BLE_H_
#define _BLE_H_

#include <zephyr/types.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>

#include "DFUManager.h"


#define DEVICE_NAME             CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN         (sizeof(DEVICE_NAME) - 1)


/*
BLE Services and Characteristics
*/
// Service-1
// Firmware
#define BT_UUID_DFU_SERV_VAL \
    BT_UUID_128_ENCODE(0x34c2e3b8, 0x34aa, 0x11eb, 0xadc1, 0x0242ac120002)
// Characteristics
// Internal Firmware
#define BT_UUID_DFU_INTERNAL_VAL \
    BT_UUID_128_ENCODE(0x34c2e3b8, 0x34ab, 0x11eb, 0xadc1, 0x0242ac120002)
// External Firmware
#define BT_UUID_DFU_EXTERNAL_VAL \
    BT_UUID_128_ENCODE(0x34c2e3b8, 0x34ac, 0x11eb, 0xadc1, 0x0242ac120002)

// Service-2
// Sensor
#define BT_UUID_SEN_SERV_VAL \
    BT_UUID_128_ENCODE(0x34c2e3b9, 0x34aa, 0x11eb, 0xadc1, 0x0242ac120002)
// Sensor Config
#define BT_UUID_SEN_CONFIG_VAL \
    BT_UUID_128_ENCODE(0x34c2e3b9, 0x34ab, 0x11eb, 0xadc1, 0x0242ac120002)


#define BT_UUID_DFU_SERV                BT_UUID_DECLARE_128(BT_UUID_DFU_SERV_VAL)
#define BT_UUID_DFU_INTERNAL            BT_UUID_DECLARE_128(BT_UUID_DFU_INTERNAL_VAL)
#define BT_UUID_DFU_EXTERNAL            BT_UUID_DECLARE_128(BT_UUID_DFU_EXTERNAL_VAL)
#define BT_UUID_SEN_SERV                BT_UUID_DECLARE_128(BT_UUID_SEN_SERV_VAL)
#define BT_UUID_SEN_CONFIG              BT_UUID_DECLARE_128(BT_UUID_SEN_CONFIG_VAL)


/*
Callbacks for BLE
*/
/** @brief Callback for device Internal Firmware*/
typedef void (*firmware_data_t)(DFUType dfuType, const void *buf, uint16_t len);
/** @brief Callback for sensor config update */
typedef void (*sensor_config_t)(const void *buf, uint16_t len);

// Callbacks struct
struct ns_cb {
    // Firmware Update
    firmware_data_t firmware_data_cb;
    // Sensor Config
    sensor_config_t sensor_config_cb;
};

/** @brief Initialize the callback functions for the BLE events
 * 
 * @note The callbacks are custom defined and are initialized to BLEHandler class
 * 
 * @param   callbacks               Struct containing pointers to functions that is to called within a callback. Can be NULL
 * 
 * @return  0, if successful. -ve error codes if failed.  
*/
int callback_init(struct ns_cb *callbacks);




#endif