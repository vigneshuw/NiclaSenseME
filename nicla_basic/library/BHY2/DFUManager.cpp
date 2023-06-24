#include <zephyr/logging/log.h>
#include <zephyr/dfu/mcuboot.h>
#include <stdio.h>

#include "DFUManager.h"
#include "BLEHandler.h"
#include "Flash/FlashFirmwareWrite.h"


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

int DFUManager::writeFirmwareToFlash(DFUType dfuType, DFULevel dfuLevel) {

    // Initialize IMG struct
    int rc = flash_img_init_id(&flash_ctx, UPLOAD_FLASH_AREA_ID);
    if(rc) {
        LOG_ERR("Flash img context cannot be initialized, rc = %d\n", rc);
    }

    if(dfuType == DFU_INTERNAL) {
        // Firmware size and params
        size_t fw_len;
        rc = spiFlash.get_file_size(_dfu_internal_fpath, &fw_len);
        if(rc || (fw_len == 0)) {
            LOG_ERR("The FW file [%s] does not exist\n", _dfu_internal_fpath);
            return rc;
        }

        // // before the start of writing. Erase the img bank
        // rc = boot_erase_img_bank(UPLOAD_FLASH_AREA_ID);
        // if(rc) {
        //     LOG_ERR("Erasing of the image bank failed! rc = %d\n", rc);
        //     return rc;
        // }

        // Allocate for read data
        uint32_t num_chunks = fw_len / CHUNK_SIZE;
        uint32_t num_chunks_remain = fw_len % CHUNK_SIZE;
        // Buf space
        uint8_t *buf = (uint8_t *) k_malloc(CHUNK_SIZE);
        memset(buf, 0, CHUNK_SIZE);

        // Write to device secondary flash
        size_t indexer = 0;
        for(size_t i = 0; i < num_chunks; i++) {
            indexer = i * 512;
            // Read from external flash
            spiFlash.littlefs_binary_read(_dfu_internal_fpath, buf, CHUNK_SIZE, indexer);
            if((num_chunks_remain == 0) && (i == (num_chunks - 1))) {
                rc = flash_img_buffered_write(&flash_ctx, buf, CHUNK_SIZE, true);
            } else {
                rc = flash_img_buffered_write(&flash_ctx, buf, CHUNK_SIZE, false);
            }
            // On Flash write fail
            if (rc)
                return rc;

        }
        // If there are remaining bytes
        if(num_chunks_remain != 0) {
            indexer += 512;
            memset(buf, 0, CHUNK_SIZE);
            spiFlash.littlefs_binary_read(_dfu_internal_fpath, buf, num_chunks_remain, indexer);
            rc = flash_img_buffered_write(&flash_ctx, buf, num_chunks_remain, true);
            if(rc)
                return rc;
        }

        // Free the buffer
        k_free(buf);

        // Check the number of bytes written
        indexer = flash_img_bytes_written(&flash_ctx);
        if(indexer != fw_len) {
            LOG_ERR("Actual FW length and written length does not match!\n");
            return -1;
        }

        // Boot upgrade request - Decide based on the level
        rc = boot_request_upgrade((dfuLevel) ? BOOT_UPGRADE_PERMANENT: BOOT_UPGRADE_TEST);
        if(rc) {
            LOG_ERR("Boot upgrade request has failed\n");
            return rc;
        }
    } else {
        LOG_WRN("DFU_EXTERNAL is not implemented!\n");
        return -2;
    }

    return 0;
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