#ifndef _BLE_HANDLER_H_
#define _BLE_HANDLER_H_

#include <zephyr/types.h>

#include "sensors/SensorTypes.h"
#include "DFUManager.h"
#include "BLE.h"


/** @brief Class of handling BLE communication for both Firmware update and Sensor Configuration */
class BLEHandler {
    public:
        BLEHandler();
        virtual ~BLEHandler();

        /** @brief Initialize the BLE Service and Characteristics. Enable Advertising and Scan Response
         * 
         * @return  true, Successful initialization of the BLE System; false, Failure in initialization of the BLE System
         * 
        */
        static bool begin();

        /** @brief Transfer data over BLE
         * 
         *  @note   NOT IMPLEMENTED
        */
        static void update();

        /** @brief End advertising and the BLE hardware
         * 
         * @return  true, on success; false, on failure.
        */
        static bool end();

        /** @brief Flag to indicate the BLE status */
        static bool bleActive;

    private:

        /** @brief If the packet transferred for the firmware is the last */
        static bool _lastDFUPack;
        /** @brief Callbacks for Read/Write events */
        static struct ns_cb app_callbacks;
        /** @brief Callbacks for BLE connect/disconnect */
        static struct bt_conn_cb conn_callbacks;

        /** @brief Method for reading and processing DFU packet 
         * 
         * @param   dfuType                 Selects the device to update firmware. DFU_INTERNAL for nrf52832, and DFU_EXTERNAL for BHI260AP
         * @param   buf                     Pointer to the data buffer from BLE callback
         * @param   len                     The length of the data transferred through BLE
        */
        static void processDFUPacket(DFUType dfuType, const void *buf, uint16_t len);

        /** @brief Callback when the central is connected */
        static void on_connected(struct bt_conn *conn, uint8_t err);
        /** @brief Callback when the central is disconnected */
        static void on_disconnected(struct bt_conn *conn, uint8_t reason);

        /** @brief Set configuration for a sensor over BLE 
         * 
         * @param   buf                     Pointer to the BLE data buffer
         * @param   len                     Number of bytes transferred over BLE
        */
        static void processSensorConfig(const void *buf, uint16_t len);

};


/** @brief The BLEHandler class can be externally linked */
extern BLEHandler bleHandler;

#endif