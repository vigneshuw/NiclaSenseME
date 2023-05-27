#ifndef _MX25R1635F_H_
#define _MX25R1635F_H_


#define MAX_PATH_LENGTH 100


class MX25R1635F {

    public:
        MX25R1635F();
        ~MX25R1635F();

        /** @brief List all the available directories and files in the give path
         * 
         * @param   path                The directory whose subdirectories and files needed to be listed
         * 
         * @retval  0 on success;
         * @retval  <0 negative error code on failure;
        */
        static int lsdir(const char *path);

        /** @brief Can increase the count of a 32 bit variable specified by the file name
         * 
         * @param   fname               The file name for the file containing the 32-bit variable
         * 
         * @retval  0 on success;
         * @retval  <0 negative error code on failure;  
        */
        static int littlefs_increase_infile_value(char *fname);

        /** @brief Perform a read on a file
         * 
         * @param   fname               The file name as a char* 
         * @param   buf                 Pointer to the buffer to store the read data
         * @param   num_bytes           Number of bytes to read from the file
         * @param   offset              An offset to the start of the read
         * 
         * @retval  0 on success;
         * @retval  <0 negative error code on failure;  
        */
        static int littlefs_binary_read(char *fname, void *buf, size_t num_bytes, size_t offset);

        /** @brief Delete a file if it exists 
         * 
         * @param   path                The path to the directory containing the file
         * @param   fname               The name of the file to be deleted
         * 
         * @retval  0 on success;
         * @retval  <0 negative error code on failure;   
        */
        static int littlefs_delete(const char *path, const char *fname);

        /** @brief Write binary data to a file 
         * 
         * @param   fname               The name of the file to write
         * @param   data                The buffer containing the data to write
         * @param   len                 Number of bytes to write
         * @param   offset              Offset to the start of the writing process
         * @param   to_unlink           True to delete the old file, if it exists
         * 
         * @retval  0 on success;
         * @retval  <0 negative error code on failure; 
         * 
         * @note    A file will be created if it does not exist.
        */
        static int littlefs_binary_write(char *fname, const unsigned char *data, size_t len, size_t offset, bool to_unlink);

        /** @brief Erase the whole flash
         * 
         * @param   id                  The ID to the flash area. Can be obtained from the device tree
         * 
         * @retval  0 on success;
         * @retval  <0 negative error code on failure; 
         * 
         * @note    Will remove the littlefs from Flash. Will unmount the drive. Needs a reboot to create the littlefs file system again
        */
        static int littlefs_flash_erase(unsigned int id);

        /** @brief Mount the flash area a littlefs 
         * 
         * @param   mp                  Mount point. Can be obtained from the device tree
         * @param   automounted         true if automount is enabled. Enabling and disabling automount can be done at the device tree
         * 
         * @retval  0 on success;
         * @retval  <0 negative error code on failure;  
        */
        static int littlefs_mount(struct fs_mount_t* mp, bool automounted);

};

/** @brief The MX25R1635F class can be externally linked */
extern MX25R1635F spiFlash;

#endif
