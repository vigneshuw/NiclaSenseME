#include <zephyr/logging/log.h>
#include <stdio.h>

#include "DFUManager.h"
#include "BLEHandler.h"


LOG_MODULE_REGISTER(MDFUManager, CONFIG_SET_LOG_LEVEL);


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
    rc = spiFlash.littlefs_mount(mp, automounted);
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
    rc = spiFlash.lsdir(mp->mnt_point);
    if (rc < 0) {
        LOG_ERR("FAIL: lsdir %s: %d\n", mp->mnt_point, rc);
        return false;
    }

    // Set the file name
    snprintf(_dfu_internal_fpath, sizeof(_dfu_internal_fpath), "%s/%s", mp->mnt_point, _dfu_internal_fname);
    snprintf(_dfu_external_fpath, sizeof(_dfu_external_fpath), "%s/%s", mp->mnt_point, _dfu_external_fname);

    // Print the file names
    LOG_DBG("Internal FW fpath %s\n", _dfu_internal_fpath);
    LOG_DBG("External FW fpath %s\n", _dfu_external_fpath);

    return true;

}

void DFUManager::processPacket(DFUType dfuType, const uint8_t *data, uint16_t len) {
    _transferPending = true;

    DFUPacket *packet = (DFUPacket *)data;
    // Update len to reflect only the data
    len = len - 5;

    LOG_DBG("Packet index -> %u\n", packet->index);

    int res;

    // Delete the old file
    if(packet->index == 0) {
        if(dfuType == DFU_INTERNAL) {
            spiFlash.littlefs_delete(mp->mnt_point, _dfu_internal_fname);
            // Reset internal CRC
            _crc_internal = 0x00;
        } else {
            spiFlash.littlefs_delete(mp->mnt_point, _dfu_external_fname);
            // Reset external CRC
            _crc_external = 0x00;
        }
    }
    // Write firmware to file system
    if (dfuType == DFU_INTERNAL) {
        res = spiFlash.littlefs_binary_write(_dfu_internal_fpath, packet->data, 
                                len, packet->index, false);
        // Compute CRC
        for(int i = 0; i < len; i++) {
            _crc_internal = _crc_internal ^ packet->data[i];
        }
    } else {
        res = spiFlash.littlefs_binary_write(_dfu_external_fpath, packet->data, 
                                len, packet->index, false);
        // Compute CRC
        for(int i = 0; i < len; i++) {
            _crc_external = _crc_external ^ packet->data[i];
        }
    }
    // Acknowledgement for a single write
    if(res) _acknowledgement = DFUAck;
        else _acknowledgement = DFUNack;

    // If the packet is the last
    if(packet->last) {
        LOG_DBG("Last packet received. Remaining: %u\n", packet->index);
    }

}

bool DFUManager::isPending() {
    return _transferPending;
}

void DFUManager::closeDfu() {
    _transferPending = false;
    // Print the CRC after data transfer
    LOG_INF("The internal and external CRC are %u, and %u", _crc_internal, _crc_external);
}

uint8_t DFUManager::acknowledgement() {
    uint8_t ack = _acknowledgement;
    // Reset after reading
    _acknowledgement = DFUNack;
    return ack;
}


DFUManager dfuManager;