#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/fs.h>

#include <stdio.h>
#include <string.h>

#include "NiclaSystem.hpp"


#define TEST_FILE_SIZE 547
#define PARTITION_NODE DT_NODELABEL(lfs1)


LOG_MODULE_REGISTER(main);


// Create the file mount point
FS_FSTAB_DECLARE_ENTRY(PARTITION_NODE);
struct fs_mount_t *mp = &FS_FSTAB_ENTRY(PARTITION_NODE);
bool automounted = false;


int main(void) {

    char fname1[MAX_PATH_LENGTH];
    char fname2[MAX_PATH_LENGTH];
    struct fs_statvfs sbuf;
    int rc;

    LOG_PRINTK("Testing the flash read and write\n");


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