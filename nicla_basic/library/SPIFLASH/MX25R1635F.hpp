#ifndef _MX25R1635F_H_
#define _MX25R1635F_H_


#define MAX_PATH_LENGTH 100


class MX25R1635F {

    public:
        MX25R1635F() {};

        static int lsdir(const char *path);
        static int littlefs_increase_infile_value(char *fname);
        static int littlefs_binary_read(char *fname, void *buf, size_t num_bytes, size_t offset);
        static int littlefs_delete(const char *path, const char *fname);
        static int littlefs_binary_write(char *fname, const unsigned char *data, size_t len, size_t offset, bool to_unlink);
        static int littlefs_flash_erase(unsigned int id);
        static int littlefs_mount(struct fs_mount_t* mp, bool automounted);

};

#endif
