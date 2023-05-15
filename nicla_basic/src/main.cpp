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
Forward Declarations
*/
int intialize_file_system(void);


/*
LittleFS
*/
// Create the file mount point
FS_FSTAB_DECLARE_ENTRY(PARTITION_NODE);
struct fs_mount_t *mp = &FS_FSTAB_ENTRY(PARTITION_NODE);
bool automounted = false;
// littlefs filesystem names
char boot_count_fname[MAX_PATH_LENGTH];
char bhi_fw_fname[MAX_PATH_LENGTH];
char bhi_fw_crc_fname[MAX_PATH_LENGTH];


/*
System Variables
*/
uint32_t boot_count;
uint8_t computed_crc = 0;
uint8_t required_crc = 0;
uint32_t fw_bin_len = 0;
uint32_t indexer = 0;

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
/** @brief Load the firmware data*/
static bool app_firmware_data_cb(const uint8_t *val, uint16_t len) {

    nicla::spiFLash.littlefs_binary_write(bhi_fw_fname, val, len, indexer, false);
    indexer += len;

    return false;
}
/** @brief Write External Status*/
static bool app_external_status_cb(uint8_t *val, uint16_t len) {
    // Parse the external data
    required_crc = val[0];

    // Length of uploaded firmware
    fw_bin_len = val[1] | (val[2] << 8) | (val[3] << 16) | (val[4] << 24);
    if(fw_bin_len != indexer) {
        LOG_WRN("Firmware lengths does not match! Required %u, Actual %u\n", fw_bin_len, indexer);
    }

    // Reset variables
    // Reset index
    indexer = 0;

    return false;
}
/** @brief Read Device Status*/
static bool app_device_status_cb(uint8_t *device_attr, uint16_t len) {
    // Placeholders
    for(int i = 0; i < len; i++) {
        device_attr[i] = i;
    }

    return false;
}

// Callbacks to the struct
static struct ns_cb app_callbacks = {
    .device_status_cb = app_device_status_cb,
    .external_status_cb = app_external_status_cb,
    .boot_cnt_cb = app_boot_cnt_cb,
    .firmware_update_cb = app_firmware_upload_cb,
    .firmware_data_cb = app_firmware_data_cb,
};

/** @brief On device connected */
static void on_connected(struct bt_conn *conn, uint8_t err) {
    if(err) {
        LOG_ERR("Connection error %d", err);
        return;
    }
    LOG_INF("Connection Established");

    // Set the LEDs to Green 
    nicla::leds.setColor(green);

}
/** @brief On device disconnected callback */
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

    /*
    Initialize Nicla System
    */
    nicla::leds.begin();
    nicla::pmic.enableCharge(100);


    /*
    File System Initialization
    */
    int rc = intialize_file_system();

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

    // Unlink the old firmware file
    rc = nicla::spiFLash.littlefs_delete(mp->mnt_point, bhi_fw_fname);
    if(rc) {
        printk("The file unlink failed %d\n", rc);
    }

    while (1) {

        k_msleep(10000);

    }

}


int intialize_file_system(void) {
    struct fs_statvfs sbuf;
    int rc;

    LOG_INF("Initializing file system on external flash\n");

    
    // Mounting the littlefs 
    if(FSTAB_ENTRY_DT_MOUNT_FLAGS(PARTITION_NODE) & FS_MOUNT_FLAG_AUTOMOUNT) {
        automounted = true;
    } else {
        automounted = false;
    }
    rc = nicla::spiFLash.littlefs_mount(mp, automounted);
    if (rc < 0) {
        LOG_ERR("Mounting failed with %d\n", rc);
        return rc;
    }
    
    // File names
    snprintf(boot_count_fname, sizeof(boot_count_fname), "%s/boot_count.bin", mp->mnt_point);
    snprintf(bhi_fw_fname, sizeof(bhi_fw_fname), "%s/bhi_update.bin", mp->mnt_point);
    snprintf(bhi_fw_crc_fname, sizeof(bhi_fw_crc_fname), "%s/bhi_update_crc.bin", mp->mnt_point);

    // Get the status of the mounted system
    rc = fs_statvfs(mp->mnt_point, &sbuf);
    if (rc < 0) {
        LOG_ERR("FAIL: statvfs: %d\n", rc);
        return rc;
    }
    LOG_INF("%s: bsize = %lu ; frsize = %lu ;"
		   " blocks = %lu ; bfree = %lu\n",
		   mp->mnt_point,
		   sbuf.f_bsize, sbuf.f_frsize,
		   sbuf.f_blocks, sbuf.f_bfree);

    // List the available directories
	rc = nicla::spiFLash.lsdir(mp->mnt_point);
	if (rc < 0) {
		LOG_ERR("FAIL: lsdir %s: %d\n", mp->mnt_point, rc);
		return rc;
	}

    rc = nicla::spiFLash.littlefs_increase_infile_value(boot_count_fname);
	if (rc) {
        LOG_ERR("FAIL: To update value in the file - %s -, err - %d", boot_count_fname, rc);
		return rc;
	}

    return 0;

}