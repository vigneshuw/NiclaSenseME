#ifndef _NICLASERVICE_H_
#define _NICLASERVICE_H_

#include <zephyr/types.h>


// Nicla Service UUID
#define BT_UUID_NS_VAL \
    BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123)

/*
Device Related
*/
// Boot Count Characteristic
#define BT_UUID_NS_BTC_VAL \
    BT_UUID_128_ENCODE(0x00001523, 0x1213, 0xefde, 0x1523, 0x785feabcd123)
// External Status
#define BT_UUID_NS_ES_VAL \
    BT_UUID_128_ENCODE(0x00001523, 0x1216, 0xefde, 0x1523, 0x785feabcd123)
// Device Status
#define BT_UUID_NS_DS_VAL \
    BT_UUID_128_ENCODE(0x00001523, 0x1217, 0xefde, 0x1523, 0x785feabcd123)

/* 
BHI260AP Firware Control
*/
// Initialize the firmware update
#define BT_UUID_NS_BHI_FU_VAL \
    BT_UUID_128_ENCODE(0x00001523, 0x1214, 0xefde, 0x1523, 0x785feabcd123)
// Transfer of firmware data
#define BT_UUID_NS_BHI_FW_DAT_VAL \
    BT_UUID_128_ENCODE(0x00001523, 0x1215, 0xefde, 0x1523, 0x785feabcd123)


#define BT_UUID_NS                  BT_UUID_DECLARE_128(BT_UUID_NS_VAL)
#define BT_UUID_NS_BTC              BT_UUID_DECLARE_128(BT_UUID_NS_BTC_VAL)
#define BT_UUID_NS_ES               BT_UUID_DECLARE_128(BT_UUID_NS_ES_VAL)
#define BT_UUID_NS_DS               BT_UUID_DECLARE_128(BT_UUID_NS_DS_VAL)

#define BT_UUID_NS_BHI_FU           BT_UUID_DECLARE_128(BT_UUID_NS_BHI_FU_VAL)
#define BT_UUID_NS_BHI_FW_DAT       BT_UUID_DECLARE_128(BT_UUID_NS_BHI_FW_DAT_VAL)


/** @brief Callback for status update*/
typedef bool (*status_t)(uint8_t *data, uint16_t len);

/** @brief Callback for reading the boot count*/
typedef uint32_t (*boot_cnt_t)(void);

/** @brief Callback for initializing firmaware update*/
typedef bool (*firmware_update_t)(const bool update_state);

/** @brief Callback for transfering firmware data*/
typedef bool (*firmware_data_t)(const uint8_t *data, uint16_t len);


/** @brief Callback struct for the NiclaSenseME Service*/
struct ns_cb {

    // Status read
    status_t device_status_cb;
    // Status write
    status_t external_status_cb;
    // Boot count callback
    boot_cnt_t boot_cnt_cb;
    // Firware update callback
    firmware_update_t firmware_update_cb;
    // Firmware data callback
    firmware_data_t firmware_data_cb;

};

/** @brief Initialize the Nicla Service
 * 
 * This function registers application callback functions with the Nicla Sense ME 
 * Service
 * 
 * @param[in]   callbacks Struct containing pointers to callback functions
 *              used by the service. This pointer can be NULL
 *              if no callback functions are defined
 * 
 * @retval      0 If the operation was successful.
 *              Otherwise, a (negative) error code is returned
 * 
*/
int nicla_service_init(struct ns_cb *callbacks);



#endif