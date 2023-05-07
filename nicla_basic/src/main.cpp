#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>

#include <stdio.h>
#include <string.h>

#include "NiclaSystem.hpp"
#include "BLE/NiclaService.hpp"


#define DEVICE_NAME                 CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN             (sizeof(DEVICE_NAME) - 1)

#define TEST_FILE_SIZE              547
#define PARTITION_NODE              DT_NODELABEL(lfs1)


LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);


/*
LittleFS
*/
// Create the file mount point
FS_FSTAB_DECLARE_ENTRY(PARTITION_NODE);
struct fs_mount_t *mp = &FS_FSTAB_ENTRY(PARTITION_NODE);
bool automounted = false;

// File names to read
char fname1[MAX_PATH_LENGTH];
char fname2[MAX_PATH_LENGTH];
uint32_t boot_count;


/*
Bluetooth Connection
*/
// Advertising parameters
static struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM((BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_IDENTITY), 
                800, 
                801, 
                NULL);
                
// Advertising and Scanning data
// Advertising data
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};
// Scanning response data
static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123))
};

// Callbacks
/** @brief Read boot count from SPI flash*/
static uint32_t app_boot_cnt_cb(void) {

    return boot_count;
}
/** @brief  Upload the firmware to the BHI260AP*/
static bool app_firmware_upload_cb(bool update_state) {
    if(update_state) {
        printk("Firmware is being uploaded......\n");
        k_msleep(10000);
        printk("Upload complete!\n");
    } else {
        printk("Firware will not be uploaded...\n");
    }

    return false;
}

// Callbacks to the struct
static struct ns_cb app_callbacks = {
    .boot_cnt_cb = app_boot_cnt_cb,
    .firmware_update_cb = app_firmware_upload_cb,
};

// On Connection callback
static void on_connected(struct bt_conn *conn, uint8_t err) {
    if(err) {
        LOG_ERR("Connection error %d", err);
        return;
    }
    LOG_INF("Connection Established");

    // Set the LEDs to Green 
    nicla::leds.setColor(green);

}

// On disconnect callback
void on_disconnected(struct bt_conn *conn, uint8_t reason) {
    LOG_INF("Disconnected. Reason %d", reason);

    nicla::leds.setColor(red);
}

// Callbacks to the struct
struct bt_conn_cb connection_callbacks = {
    .connected      = on_connected,
    .disconnected   = on_disconnected,
};


int main(void) {

    nicla::leds.begin();
    nicla::pmic.enableCharge(100);

    struct fs_statvfs sbuf;
    int rc;

    LOG_INF("Testing the flash read and write and BLE\n");


    if(FSTAB_ENTRY_DT_MOUNT_FLAGS(PARTITION_NODE) & FS_MOUNT_FLAG_AUTOMOUNT) {
        automounted = true;
    } else {
        automounted = false;
    }
    rc = nicla::spiFLash.littlefs_mount(mp, automounted);
    if (rc < 0) {
        return 0;
    }

    snprintf(fname1, sizeof(fname1), "%s/boot_count", mp->mnt_point);
    snprintf(fname2, sizeof(fname2), "%s/pattern.bin", mp->mnt_point);

    rc = fs_statvfs(mp->mnt_point, &sbuf);
    if (rc < 0) {
        LOG_PRINTK("FAIL: statvfs: %d\n", rc);
        goto out;
    }

    LOG_PRINTK("%s: bsize = %lu ; frsize = %lu ;"
		   " blocks = %lu ; bfree = %lu\n",
		   mp->mnt_point,
		   sbuf.f_bsize, sbuf.f_frsize,
		   sbuf.f_blocks, sbuf.f_bfree);

	rc = nicla::spiFLash.lsdir(mp->mnt_point);
	if (rc < 0) {
		LOG_PRINTK("FAIL: lsdir %s: %d\n", mp->mnt_point, rc);
		goto out;
	}

    rc = nicla::spiFLash.littlefs_increase_infile_value(fname1);
	if (rc) {
		goto out;
	}

    /*
    BLE
    */
    // Registering Callbacks
    int err;
    bt_conn_cb_register(&connection_callbacks);
    err = nicla_service_init(&app_callbacks);
    if(err) {
        LOG_ERR("Failed to initialize the read/write calbacks\n");
        return 1;
    }
    
    // Enable BLE
    err = bt_enable(NULL);
    if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return 1;
	}
	LOG_INF("Bluetooth initialized");

    // Start Advertising
	err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
		return 1;
	}
	LOG_INF("Advertising successfully started"); 

    while (1) {
        // Read the boot count and print
        nicla::spiFLash.littlefs_binary_read(fname1, &boot_count, sizeof(boot_count));
        LOG_PRINTK("Boot count is %u\n", boot_count);
        k_msleep(5000);
    }


out:
	// rc = fs_unmount(mp);
	// LOG_PRINTK("%s unmount: %d\n", mp->mnt_point, rc);
	return 0;

}