#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <zephyr/dfu/flash_img.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/dfu/mcuboot.h>
#include "NiclaFW/nicla_fw.h"


// FW files
#define nicla_fw                app_update_bin
#define nicla_fw_len            app_update_bin_len

// Get uploadable data size
uint32_t chunks = nicla_fw_len / 512;
uint32_t chunks_remain = nicla_fw_len % 512;
// FW chunk data buffer
uint8_t buf[512];

// Node label
#define UPLOAD_FLASH_AREA_LABEL             slot1_partition
// Flash area ID
#define UPLOAD_FLASH_AREA_ID                FIXED_PARTITION_ID(UPLOAD_FLASH_AREA_LABEL)
// Flash area controller
#define UPLOAD_FLASH_AREA_CONTROLLER        DT_GPARENT(DT_NODELABEL(UPLOAD_FLASH_AREA_LABEL))


LOG_MODULE_REGISTER(main, CONFIG_SET_LOG_LEVEL);


struct flash_img_context flash_ctx;
struct mcuboot_img_header header;

int main(void) {
    
    // Flash IMG init
    int rc = flash_img_init_id(&flash_ctx, UPLOAD_FLASH_AREA_ID);
    if (rc) {
        printk("Failed to init flash img, rc = %d\n", rc);
    }

    // Compute the CRC
    uint8_t original_crc = 0x00;
    for(unsigned int i = 0; i < nicla_fw_len; i++) {
        original_crc = original_crc ^ nicla_fw[i];
    }
    printk("The Original CRC is %u\n", original_crc);

    // rc = flash_img_buffered_write(&flash_ctx, nicla_fw, nicla_fw_len, true);
    // if(rc) {
    //     printk("Failed to write on flash, rc = %d", rc);
    // }
    
    // Go through chunk by chunk and write to flash
    size_t indexer = 0;
    for(size_t i = 0; i < chunks; i++) {

        indexer = i * 512;

        bytecpy(buf, &nicla_fw[indexer], 512);
        // Write the buf to flash
        rc = flash_img_buffered_write(&flash_ctx, buf, 512, false);
        if(rc) {
            printk("Failed to write on flash, rc = %d", rc);
        }
    }
    indexer += 512;
    memset(buf, 0, chunks_remain);
    // Write the remainder
    bytecpy(buf, &nicla_fw[indexer], chunks_remain);
    // Write the buf to flash
    rc = flash_img_buffered_write(&flash_ctx, buf, chunks_remain, true);
    if(rc) {
        printk("Failed to write on flash, rc = %d", rc);
    }

    // Print the number of bytes written
    size_t bytes_written = flash_img_bytes_written(&flash_ctx);
    printk("The total number of bytes written is %u\n", bytes_written);

    // // Erasing the image bank 
    // rc = boot_erase_img_bank(UPLOAD_FLASH_AREA_ID);
    // if(rc) {
    //    printk("Failed to boot erase on flash, rc = %d\n", rc); 
    // } else {
    //     printk("Erase successful\n");
    // }

    // Read the header information for an image
    rc = boot_read_bank_header(UPLOAD_FLASH_AREA_ID, &header, sizeof(header));
    if(rc) {
        printk("Failed to read the image header, rc = %d\n", rc);
    }

    // Start this new image
    rc = boot_request_upgrade(BOOT_UPGRADE_TEST);
    if(rc) {
        printk("Boot upgrade request has failed\n");
    } else {
        printk("Boot upgrade request is successful\n");
    }
    while (1) {
        
        k_msleep(5000);
        // Print image information
        printk("Image Magic, %u\n", header.mcuboot_version);

    }

}
