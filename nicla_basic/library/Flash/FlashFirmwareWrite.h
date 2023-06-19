#ifndef _FLASH_FIRMWARE_WRITE_H
#define _FLASH_FIRMWARE_WRITE_H

#include <zephyr/types.h>
#include <zephyr/dfu/flash_img.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/storage/flash_map.h>

// Code partition label
#define UPLOAD_FLASH_AREA_LABEL             mcuboot_secondary
// Flash area ID
#define UPLOAD_FLASH_AREA_ID                FIXED_PARTITION_ID(UPLOAD_FLASH_AREA_LABEL)
// Flash Controller
#define UPLOAD_FLASH_AREA_CONTROLLER        DT_GPARENT(DT_NODELABEL(UPLOAD_FLASH_AREA_LABEL))
// FW write chunk size
#define CHUNK_SIZE                          512

// Flash img
struct flash_img_context flash_ctx;
struct mcuboot_img_header img_header;


#endif