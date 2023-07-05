#include <zephyr/types.h>

#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>


// Service-3
// Sensor data transport service
#define BT_UUID_SEN_DATA_SERV_VAL \
    BT_UUID_128_ENCODE(0x34c2e3c8, 0x34aa, 0x11eb, 0xadc1, 0x0242ac120002)
// Data transfer notify
#define BT_UUID_SEN_DATA_VAL \
    BT_UUID_128_ENCODE(0x34c2e3c8, 0x34ab, 0x11eb, 0xadc1, 0x0242ac120002)

#define BT_UUID_SEN_DATA_SERV           BT_UUID_DECLARE_128(BT_UUID_SEN_DATA_SERV_VAL)
#define BT_UUID_SEN_DATA                BT_UUID_DECLARE_128(BT_UUID_SEN_DATA_VAL)


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
int sensor_sensor_data_notify(uint8_t *data_buf);