#include <zephyr/types.h>

#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>


/*
BLE Services and Characteristics
*/
// Service-3
// Sensor data transport service
#define BT_UUID_SEN_DATA_SERV_VAL \
    BT_UUID_128_ENCODE(0x34c2e3c8, 0x34aa, 0x11eb, 0xadc1, 0x0242ac120002)
// Characteristics
// Data transfer notify
#define BT_UUID_SEN_DATA_VAL \
    BT_UUID_128_ENCODE(0x34c2e3c8, 0x34ab, 0x11eb, 0xadc1, 0x0242ac120002)
// Sensor Selection
#define BT_UUID_SEN_SEL_VAL \
    BT_UUID_128_ENCODE(0x34c2e3c8, 0x34ac, 0x11eb, 0xadc1, 0x0242ac120002)


#define BT_UUID_SEN_DATA_SERV           BT_UUID_DECLARE_128(BT_UUID_SEN_DATA_SERV_VAL)
#define BT_UUID_SEN_DATA                BT_UUID_DECLARE_128(BT_UUID_SEN_DATA_VAL)
#define BT_UUID_SEN_SEL                 BT_UUID_DECLARE_128(BT_UUID_SEN_SEL_VAL)


/*
Callbacks for BLE
*/
/** @brief  Callback for sensor selection for datastream */
typedef void (*sensor_select_t)(uint8_t sensor_id);


// Callback struct
struct ns_sd_cb {
    sensor_select_t sensor_select_cb;
};


/** @brief Send the sensor value as a notification
 * 
 * This function sends the sensor data to all the connected peers
 * 
 * @param   data_packet             The data packet that is to be sent,
 * 
 * @retval  0   If successful.
 * @retval  Otherwise, a -ve error code
 * 
*/
int sensor_send_data_notify(uint8_t *data_buf, uint16_t len);

/** @brief Initialize the callback functions for the Sensor Select BLE event
 * 
 * @note The callbacks are custom defined and are initialized to main class
 * 
 * @param   callbacks               Struct containing pointers to functions that is to called within a callback. Can be NULL
 * 
 * @retval  0   If successful.
 * @retval      Otherwise, a -ve error codes if failed.  
*/
int sensor_data_callback_init(struct ns_sd_cb *callbacks);