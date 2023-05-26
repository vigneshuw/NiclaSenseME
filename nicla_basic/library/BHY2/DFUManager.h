#ifndef _DFU_MANAGER_H_
#define _DFU_MANAGER_H_

#include <zephyr/types.h>
#include <zephyr/fs/fs.h>

#include "NiclaSystem.hpp"

#define PARTITION_NODE              DT_NODELABEL(lfs1)


/** @brief Type of firmware, nrf52832 or BHI260*/
enum DFUType {
    DFU_INTERNAL,
    DFU_EXTERNAL
};

/** @brief Acknowledgement*/
enum DFUAckCode {
    DFUAck = 0x0F,
    DFUNack = 0x00
};

/** @brief The structure for the firmware data packet*/
struct __attribute__((packed)) DFUPacket {
    uint8_t last: 1;
    union {
        uint16_t index: 15;
        uint16_t remaining: 15;
    };
    uint8_t data[200];
};

/** @brief Class for handling the DFU for both nrf52832 and BHI260*/
class DFUManager{
    public:
        DFUManager();
        virtual ~DFUManager();

        /** @brief Initialize and mount the littlefs file system 
         * 
         * @return true, file system mounted and readable; false, file sytem unmountable ro unreadable
         * 
        */
        bool begin();

        void processPacket(DFUType dfuType, const uint8_t *data);

        void closeDfu();

        bool isPending();

        uint8_t acknowledgement();

    private:

        uint8_t _acknowledgement;
        bool _transferPending;

        char _dfu_internal_fname[MAX_PATH_LENGTH];
        char _dfu_external_fname[MAX_PATH_LENGTH];

    private:

        friend class BHY2;

};


/** @brief The DFUManager class can be externally linked to as dfuManager */
extern DFUManager dfuManager;

#endif