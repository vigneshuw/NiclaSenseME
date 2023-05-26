#include <zephyr/logging/log.h>
#include <stdio.h>

#include "DFUManager.h"
#include "BLEHandler.h"


/*
LittleFS
*/
FS_FSTAB_DECLARE_ENTRY(PARTITION_NODE);
struct fs_mount_t *mp = &FS_FSTAB_ENTRY(PARTITION_NODE);


DFUManager::DFUManager() : _acknowledgement(DFUNack), _transferPending(false) {

}

DFUManager::~DFUManager() 
{

}

bool DFUManager::begin() {
    bool automounted = false;

    struct fs_statvfs sbuf;
    int rc;

    // Mounting
    if(FSTAB_ENTRY_DT_MOUNT_FLAGS(PARTITION_NODE) & FS_MOUNT_FLAG_AUTOMOUNT) {
        automounted = true;
    } else {
        automounted = false;
    }
    rc = nicla::spiFLash.littlefs_mount(mp, automounted);
    if(rc < 0) {
        LOG_ERR("Mounting failed with %d\n", rc);
        return false;
    }

    // File system details
    rc = fs_statvfs(mp->mnt_point, &sbuf);
    if (rc < 0) {
        LOG_ERR("FAIL: statvfs: %d\n", rc);
        return false;
    }
    LOG_DBG("%s: bsize = %lu ; frsize = %lu ;"
		   " blocks = %lu ; bfree = %lu\n",
		   mp->mnt_point,
		   sbuf.f_bsize, sbuf.f_frsize,
		   sbuf.f_blocks, sbuf.f_bfree);

    // List all available directories
    rc = nicla::spiFLash.lsdir(mp->mnt_point);
    if (rc < 0) {
        LOG_ERR("FAIL: lsdir %s: %d\n", mp->mnt_point, rc);
        return false;
    }

    // Set the file name
    snprintf(_dfu_internal_fname, sizeof(_dfu_internal_fname), "%s/NRF52_UPDATE.BIN", mp->mnt_point);
    snprintf(_dfu_external_fname, sizeof(_dfu_external_fname), "%s/BHY_UPDATE.BIN", mp->mnt_point);

    return true;

}

void DFUManager::processPacket(DFUType dfuType, const uint8_t *data) {
    _transferPending = true;

    DFUPacket *packet = (DFUPacket *)data;

    LOG_DBG("Packet index -> %u\n", packet->index);

    if(packet->index == 0) {
        // Unlink the old file
        
    }

    if (dfuType == DFU_INTERNAL) {
            nicla::spiFLash.littlefs_binary_write(_dfu_external_fname, packet->data, sizeof(packet->data), packet->index, false)
        }


}

bool DFUManager::isPending() {
    return _transferPending;
}

void DFUManager::closeDfu() {
    _transferPending = false;
}

uint8_t DFUManager::acknowledgement() {
    uint8_t ack = _acknowledgement;
    // Reset after reading
    _acknowledgement = DFUNack;
    return ack;
}

