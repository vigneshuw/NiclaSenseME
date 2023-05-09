#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>

#include <stdio.h>
#include <string.h>

#include "NiclaSystem.hpp"
#include "BHIFW/fw.h"


// The littlefs file system
#define PARTITION_NODE              DT_NODELABEL(lfs1)
// BHI firmware
#define fw_bin                      BHI260AP_NiclaSenseME_flash_fw
#define fw_bin_len                  BHI260AP_NiclaSenseME_flash_fw_len


LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);


/*
LittleFS
*/
// Create the file mount point
FS_FSTAB_DECLARE_ENTRY(PARTITION_NODE);
struct fs_mount_t *mp = &FS_FSTAB_ENTRY(PARTITION_NODE);
bool automounted = false;

// File names to read
char boot_count_fname[MAX_PATH_LENGTH];
char bhi_firmware_fname[MAX_PATH_LENGTH];
char crc_firmware_fname[MAX_PATH_LENGTH];
// Variables for writing
uint32_t boot_count;
uint8_t original_crc = 0;
uint8_t flash_crc = 0;

// Reading the written firmware
// Split and read
const uint8_t divider = 200;
uint8_t buf[divider];
uint32_t mod_length = fw_bin_len / divider;
uint32_t remain_length = fw_bin_len % divider;

int main(void) {

    // Enable LEDs and charging
    nicla::leds.begin();
    nicla::pmic.enableCharge(100);

    struct fs_statvfs sbuf;
    int rc;

    LOG_INF("Uploading the BHI fimware to SPI Flash\n");

    // Mouting the littlefs filesystem
    if(FSTAB_ENTRY_DT_MOUNT_FLAGS(PARTITION_NODE) & FS_MOUNT_FLAG_AUTOMOUNT) {
        automounted = true;
    } else {
        automounted = false;
    }
    rc = nicla::spiFLash.littlefs_mount(mp, automounted);
    if (rc < 0) {
        return 0;
    }

    // Filenames to write
    snprintf(boot_count_fname, sizeof(boot_count_fname), "%s/boot_count.bin", mp->mnt_point);
    snprintf(bhi_firmware_fname, sizeof(bhi_firmware_fname), "%s/bhi_update.bin", mp->mnt_point);
    snprintf(crc_firmware_fname, sizeof(crc_firmware_fname), "%s/bhi_update_crc.bin", mp->mnt_point);
    // Status of the mount point
    rc = fs_statvfs(mp->mnt_point, &sbuf);
    if (rc < 0) {
        LOG_PRINTK("FAIL: statvfs: %d\n", rc);
        goto out;
    }
    // Print the flash properties
    LOG_PRINTK("%s: bsize = %lu ; frsize = %lu ;"
		   " blocks = %lu ; bfree = %lu\n",
		   mp->mnt_point,
		   sbuf.f_bsize, sbuf.f_frsize,
		   sbuf.f_blocks, sbuf.f_bfree);

    // List the available files in the flash
	rc = nicla::spiFLash.lsdir(mp->mnt_point);
	if (rc < 0) {
		LOG_PRINTK("FAIL: lsdir %s: %d\n", mp->mnt_point, rc);
		goto out;
	}
    
    // Increment boot count and write to flash
    rc = nicla::spiFLash.littlefs_increase_infile_value(boot_count_fname);
	if (rc) {
		goto out;
	}

    /*
    Firmware write to flash
    */
    // The current CRC value
    original_crc = 0;
    for (unsigned int i = 0; i < fw_bin_len; i++) {
        original_crc = original_crc ^ fw_bin[i];
    }
    printk("The Original CRC is %u\n", original_crc);
    // Write the firmware
    rc = nicla::spiFLash.littlefs_binary_write(bhi_firmware_fname, fw_bin, fw_bin_len);
    if (rc) {
        goto out;
    }
    printk("Firmware write was successful!!\n");

    // Read the written firmware to compare CRC
    flash_crc = 0;
    for(unsigned int i = 0; i < mod_length; i++) {
        rc = nicla::spiFLash.littlefs_binary_read(bhi_firmware_fname, &buf, divider, i * divider);
        if(rc) {
            goto out;
        }
        for (unsigned int j = 0; j < divider; j++) {
            flash_crc = flash_crc ^ buf[j];
        }

    }
    rc = nicla::spiFLash.littlefs_binary_read(bhi_firmware_fname, &buf, remain_length, (mod_length * divider));
    for (unsigned int j = 0; j < remain_length; j++) {
            flash_crc = flash_crc ^ buf[j];
    }
    printk("The Flash CRC is %u\n", flash_crc);

    // Check to see if firmware match
    if(original_crc == flash_crc) {
        printk("The Original CRC and Flash CRC match!\n");
        rc = nicla::spiFLash.littlefs_binary_write(crc_firmware_fname, &original_crc, 1);
        if(rc) {
            goto out;
        }
        printk("The Original CRC %u is written to flash\n", original_crc);
    }

    

    while (1) {
        // Read the boot count and print
        // nicla::spiFLash.littlefs_binary_read(fname1, &boot_count, sizeof(boot_count));
        // LOG_PRINTK("Boot count is %u\n", boot_count);
        k_msleep(10000);
    }


out:
	// rc = fs_unmount(mp);
	// LOG_PRINTK("%s unmount: %d\n", mp->mnt_point, rc);
	return 0;

}