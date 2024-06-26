#ifndef _DFU_MANAGER_H_
#define _DFU_MANAGER_H_

#include <zephyr/types.h>
#include <zephyr/fs/fs.h>

#include "SPIFLASH/MX25R1635F.hpp"

#define PARTITION_NODE              DT_NODELABEL(lfs1)
#define BHI_MAX_WRITE_LEN           256


/** @brief Type of firmware, nrf52832 or BHI260*/
enum DFUType {
    DFU_INTERNAL,
    DFU_EXTERNAL
};

/** @brief Type of frimware update, test or permanent */
enum DFULevel {
    DFU_TEST,
    DFU_PERMANENT
};

/** @brief Acknowledgement*/
enum DFUAckCode {
    DFUAck = 0x0F,
    DFUNack = 0x00
};

/** @brief The structure for the firmware data packet*/
struct __attribute__((packed)) DFUPacket {
    uint8_t last: 1;
    uint32_t index;
    uint8_t data[200];
};

/** @brief Class for handling the DFU for both nrf52832 and BHI260*/
class DFUManager{
    public:
        DFUManager();
        virtual ~DFUManager();

        /** @brief Initialize and mount the littlefs file system. Set file names 
         * 
         * @return true, file system mounted and readable; false, file sytem unmountable ro unreadable
         * 
        */
        bool begin();

        void processPacket(DFUType dfuType, const uint8_t *data, uint16_t len);

        int writeFirmwareToFlash(DFUType dfuType, DFULevel dfuLevel);

        /** @brief Ensures the FW update is complete */
        void closeDfu();

        /** @brief Check the status of device firmware transfer 
         * 
         * @return  true, if transfer is on-going; false, if NO transfer is on-going
        */
        bool isPending();

        /** @brief Get the acknowledgement for transfer and then reset it
         * 
         * @return  DFUAck or DFUNack
        */
        uint8_t acknowledgement();

    private:
        /** @brief Packet write on littlefs file system update*/
        uint8_t _acknowledgement;
        /** @brief State of the transfer. 0x0F, if a transfer is on-going; 0x00, if there is not transfer*/
        bool _transferPending;

        // CRC
        uint8_t _crc_internal = 0x00;
        uint8_t _crc_external = 0x00;

        // File paths
        char _dfu_internal_fpath[MAX_PATH_LENGTH];
        char _dfu_external_fpath[MAX_PATH_LENGTH];
        // File names
        const char *_dfu_internal_fname = "NRF52_UPDATE.BIN";
        const char *_dfu_external_fname = "BHY_UPDATE.BIN";

        // BHI260 FW Update functions
        size_t get_update_file_size();
        int8_t upload_firmware();

    private:
        friend class BHY2;

};


/** @brief The DFUManager class can be externally linked to as dfuManager */
extern DFUManager dfuManager;

#endif