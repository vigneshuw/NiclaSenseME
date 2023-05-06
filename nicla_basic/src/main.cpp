#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>
// BLE Connection
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/addr.h>

#include <stdio.h>
#include <string.h>

#include "NiclaSystem.hpp"


#define DEVICE_NAME                 CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN             (sizeof(DEVICE_NAME) - 1)
#define TEST_FILE_SIZE              547
#define PARTITION_NODE              DT_NODELABEL(lfs1)


LOG_MODULE_REGISTER(main);


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


// Create the file mount point
FS_FSTAB_DECLARE_ENTRY(PARTITION_NODE);
struct fs_mount_t *mp = &FS_FSTAB_ENTRY(PARTITION_NODE);
bool automounted = false;


int main(void) {

    char fname1[MAX_PATH_LENGTH];
    char fname2[MAX_PATH_LENGTH];
    struct fs_statvfs sbuf;
    int rc;

    LOG_INF("Testing the flash read and write\n");


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

    uint32_t data; 
    while (1) {
        // Read the boot count and print
        nicla::spiFLash.littlefs_binary_read(fname1, &data, sizeof(data));
        LOG_PRINTK("Boot count is %u\n", data);
        k_msleep(5000);
    }


out:
	// rc = fs_unmount(mp);
	// LOG_PRINTK("%s unmount: %d\n", mp->mnt_point, rc);
	return 0;

}