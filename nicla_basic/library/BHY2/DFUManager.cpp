#include <zephyr/logging/log.h>
#include <zephyr/dfu/mcuboot.h>
#include <stdio.h>

#include "DFUManager.h"
#include "BLEHandler.h"
#include "BoschSensortec.h"
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
            indexer = i * CHUNK_SIZE;
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
            indexer += CHUNK_SIZE;
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

        // Reset the System
        k_msleep(2000);
        NVIC_SystemReset();

    } else {
        
        // BHY2 API status tracker
        int8_t rslt;

        // Reset the BHY2 device
        rslt = sensortec.soft_reset_bhy2_device();
        sensortec.print_api_error(rslt);

        // Configure host interrupt control
        rslt = sensortec.update_host_interrupt_ctrl(0);
        sensortec.print_api_error(rslt);
        // Configure host interface control
        rslt = sensortec.update_host_interface_ctrl(0);
        sensortec.print_api_error(rslt);

        // Get the boot status
        uint8_t boot_status = sensortec.get_boot_status();
        LOG_DBG("Before FW Update, Boot status: %02X", boot_status);
        
        // Erase flash sectors for loading firmware
        if(boot_status & BHY2_BST_FLASH_DETECTED) {

            uint32_t start_addr = BHY2_FLASH_SECTOR_START_ADDR;
            size_t fw_update_len = get_update_file_size();
            if(fw_update_len < 1){
                LOG_ERR("Fail to update firmware; The firmware length is less than 1");
                return -1;
            } else {
                LOG_DBG("Flash detected and BHY2 firmware update is available. Erasing will start!");
                uint32_t end_addr = start_addr + fw_update_len;
                rslt = sensortec.erase_bhy2_flash(start_addr, end_addr);
                sensortec.print_api_error(rslt);
            }


        } else {
            LOG_ERR("BHY2 Flash not detected for update process");
            return -1;
        }

        // Upload the FW to Flash
        rslt = upload_firmware();
        sensortec.print_api_error(rslt);

        // Check if the sensor is ready to load firmware
        boot_status = sensortec.get_boot_status();
        LOG_DBG("After FW Update, Boot status: %02X", boot_status);


        uint8_t sensor_error = sensortec.get_bhy2_error_value();
        if(sensor_error) {
            LOG_ERR("BHY2 Error - %s", get_sensor_error_text(sensor_error));
        }

        // Re-initialize the BHY2 Sensor
        sensortec.begin();

        // Get the new version
        uint16_t kernel_version = sensortec.get_bhy2_kernel_version();
        LOG_DBG("The kernel version after update is %u", kernel_version);


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

size_t DFUManager::get_update_file_size() {
    size_t fw_len = 0;

    // Read the fw file size
    int rc = spiFlash.get_file_size(_dfu_external_fpath, &fw_len);
    if(rc || (fw_len == 0)) {
        LOG_ERR("The FW file [%s] does not exist\n", _dfu_external_fpath);
        return rc;
    }

    return fw_len;
}

int8_t DFUManager::upload_firmware() {

    int8_t rslt = BHY2_OK;

    // Get the FW length
    size_t fw_len = DFUManager::get_update_file_size();
    if(fw_len < 1) {
        return BHY2_E_NULL_PTR;
    }

    // Iterations
    uint16_t iterations = fw_len / BHI_MAX_WRITE_LEN;
    uint32_t num_iterations_remain = fw_len % BHI_MAX_WRITE_LEN;
    // Buf space
    uint8_t *buf = (uint8_t *) k_malloc(BHI_MAX_WRITE_LEN);
    memset(buf, 0, BHI_MAX_WRITE_LEN);

    // Write to BHY2 device
    size_t indexer = 0;
    for(size_t i = 0; (i < iterations) && (rslt == BHY2_OK); i++) {
        indexer = i * BHI_MAX_WRITE_LEN;
        // Read from external flash
        spiFlash.littlefs_binary_read(_dfu_external_fpath, buf, BHI_MAX_WRITE_LEN, indexer);

        // Write to flash partly
        rslt = sensortec.upload_firmware_to_flash_partly(buf, indexer, BHI_MAX_WRITE_LEN);

        // Last run of the loop
        if(i == (iterations - 1)) {
            // The remaining packets
            if(num_iterations_remain != 0) {
                indexer += BHI_MAX_WRITE_LEN;
                memset(buf, 0, BHI_MAX_WRITE_LEN);
                spiFlash.littlefs_binary_read(_dfu_external_fpath, buf, num_iterations_remain, indexer);

                // Higher 4 bytes alignment
                if((num_iterations_remain % 4) != 0) {
                    num_iterations_remain = ((num_iterations_remain >> 2) + 1) << 2;
                }

                // Write the remaining items to bhy2 flash
                rslt = sensortec.upload_firmware_to_flash_partly(buf, indexer, num_iterations_remain);

            }
        }
    }

    return rslt;

}


DFUManager dfuManager;