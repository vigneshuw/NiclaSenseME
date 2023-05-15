#include <zephyr/fs/fs.h>
#include <zephyr/logging/log.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/storage/flash_map.h>
#include "MX25R1635F.hpp"


LOG_MODULE_REGISTER(MX25R1635F, LOG_LEVEL_WRN);


int MX25R1635F::lsdir(const char* path) {
    int res;
    struct fs_dir_t dirp;
    static struct fs_dirent entry;

    fs_dir_t_init(&dirp);

    // Verify opening directory
    res = fs_opendir(&dirp, path);
    if (res) {
        LOG_ERR("Error opening dir %s [%d]\n", path, res);
        return res;
    }     

    LOG_PRINTK("\nListing dir %s ...\n", path);
    for(;;) {
        // Read directory entries
        res = fs_readdir(&dirp, &entry);

        if (res || entry.name[0] == 0) {
            if (res < 0) {
                LOG_ERR("Error reading dir [%d]\n", res);
            }
            break;
        }

        // File or directory
        if (entry.type == FS_DIR_ENTRY_DIR) {
            LOG_PRINTK("[DIR] %s\n", entry.name);
        } else {
            LOG_PRINTK("[FILE] %s (size = %zu)\n", 
                entry.name, entry.size);
        }
    }

    // Verify closing directory
    fs_closedir(&dirp);
    return res;

}


int MX25R1635F::littlefs_binary_read(char* fname, void* buf, size_t num_bytes, size_t offset) {

    struct fs_file_t file;
    int rc, ret;

    fs_file_t_init(&file);
    rc = fs_open(&file, fname, FS_O_CREATE | FS_O_RDWR);
    if (rc < 0) {
        LOG_ERR("FAIL: open %s: %d", fname, rc);
    }
    
    // Read offset
    rc = fs_seek(&file, offset, FS_SEEK_SET);
    if (rc < 0) {
        LOG_ERR("FAIL: seek %s: %d", fname, rc);
        goto out;
    }

    rc = fs_read(&file, buf, num_bytes);
    if (rc < 0) {
        LOG_ERR("FAIL: read %s: %d", fname, rc);
        goto out;
    }

out:
    ret = fs_close(&file);
    if (ret < 0) {
        LOG_ERR("FAIL: close %s:%d", fname, ret);
        return ret;
    }

    return (rc < 0 ? rc : 0);

} 


int MX25R1635F::littlefs_increase_infile_value(char* fname) {
    uint32_t counter = 0;
    struct fs_file_t file;
    int rc, ret;

    fs_file_t_init(&file);
    rc = fs_open(&file, fname, FS_O_CREATE | FS_O_RDWR);
    if(rc < 0) {
        LOG_ERR("FAIL: open %s: %d", fname, rc);
    }

    rc = fs_read(&file, &counter, sizeof(counter));
    if(rc < 0) {
        LOG_ERR("FAIL: read %s: [rd:%d]", fname, rc);
		goto out;
    }
    LOG_PRINTK("%s read count:%u (bytes: %d)\n", fname, counter, rc);
    
    rc = fs_seek(&file, 0, FS_SEEK_SET);
    if (rc < 0) {
        LOG_ERR("FAIL: seek %s: %d", fname, rc);
        goto out;
    }

    // Increase the counter
    counter += 1;
    rc = fs_write(&file, &counter, sizeof(counter));
    if (rc < 0) {
        LOG_ERR("FAIL: write %s: %d", fname, rc);
        goto out;
    }

out:
    ret = fs_close(&file);
    if (ret < 0) {
        LOG_ERR("FAIL: close %s:%d", fname, ret);
        return ret;
    }

    return (rc < 0 ? rc : 0);


}


int MX25R1635F::littlefs_flash_erase(unsigned int id) {

    const struct flash_area *pfa;
    int rc;

    rc = flash_area_open(id, &pfa);
    if(rc < 0) {
        LOG_ERR("FAIL: unable to find flash area %u: %d\n",
			id, rc);
		return rc;
    }

    LOG_PRINTK("Area %u at 0x%x on %s for %u bytes\n", 
            id, (unsigned int)pfa->fa_off, pfa->fa_dev->name, 
            (unsigned int) pfa->fa_size);

    // Wipe the flash area
    if (IS_ENABLED(CONFIG_APP_WIPE_STORAGE)) {
        rc = flash_area_erase(pfa, 0, pfa->fa_size);
        if (rc < 0) {
            LOG_ERR("FAIL: Erasing flash area %d\n", rc);
        }
    }

    flash_area_close(pfa);
    return rc;
}


int MX25R1635F::littlefs_mount(struct fs_mount_t* mp, bool automounted) {
    int rc;

    rc = littlefs_flash_erase((uintptr_t)mp->storage_dev);
    if (rc < 0) {
        return rc;
    }

    // Mount only if it is not automounted
    if(automounted) {
        LOG_PRINTK("%s automounted\n", mp->mnt_point);
    } else {
        rc = fs_mount(mp);
        if (rc < 0) {
            LOG_PRINTK("FAIL: mount id %" PRIuPTR " at %s: %d\n",
		       (uintptr_t)mp->storage_dev, mp->mnt_point, rc);
		    return rc;
        }
        LOG_PRINTK("%s mount: %d\n", mp->mnt_point, rc);
    }

    return 0;

}


int MX25R1635F::littlefs_delete(char* fname) {

    // Delete file
    int rc = fs_unlink(fname);

    return rc;

}


int MX25R1635F::littlefs_binary_write(char* fname, const unsigned char* data, size_t len, size_t offset=0, bool to_unlink=false) {

    struct fs_dirent dirent;
    struct fs_file_t file;
    int rc, ret;

    // Delete old only when required
    if(to_unlink) {
        // Delete the old
        fs_unlink(fname);
    }
    

    fs_file_t_init(&file);

    // Open
    rc = fs_open(&file, fname, FS_O_CREATE | FS_O_RDWR);
    if (rc < 0) {
        LOG_ERR("FAIL: open %s: %d\n", fname, rc);
        return rc;
    }

    rc = fs_stat(fname, &dirent);
    if (rc < 0) {
		LOG_ERR("FAIL: stat %s: %d\n", fname, rc);
		goto out;
	}

    // Check for file and write the binary data
    if(rc == 0 && dirent.type == FS_DIR_ENTRY_FILE && dirent.size == 0) {
        LOG_INF("File: %s not found, create one!", 
                fname);
    }
    // Write the binary data
    rc = fs_seek(&file, offset, FS_SEEK_SET);
    if (rc < 0) {
        LOG_ERR("FAIL: seek %s: %d\n", fname, rc);
		goto out;
    }

    rc = fs_write(&file, data, len);
    if (rc < 0) {
        LOG_ERR("FAIL: write %s: %d\n", fname, rc);
    }

out:
    ret = fs_close(&file);
    if (ret < 0) {
        LOG_ERR("FAIL: close %s: %d\n", fname, ret);
        return ret;
    }

    return (rc < 0 ? rc : 0);

}