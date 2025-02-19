#pragma once

#include <zephyr/kernel.h>

#include <zephyr/storage/disk_access.h>

#include <zephyr/fs/fs.h>

#include <ff.h>

#define DISK_DRIVE_NAME "SD"

#define DISK_MOUNT_PT "/"DISK_DRIVE_NAME":"

#define CHUNK_SIZE 1024

class CFileIoWrapper
{

private:

    // File I/O wrapper class

    // This class is a wrapper for file I/O operations. It is used to abstract
    // the file I/O operations from the rest of the application. This allows
    // the application to be more easily ported to different platforms.

    // The class provides a simple interface for opening, reading, writing, and
    // closing files. It also provides a way to check if a file exists, and to
    // get the size of a file.

    // The class is implemented using the C standard I/O library, which provides
    // a portable way to perform file I/O operations.

    // The class is designed

    static CFileIoWrapper* m_instance;

    struct k_mutex my_mutex;

    static FATFS fat_fs;

    static fs_mount_t mp;

    struct fs_file_t file;

    uint64_t memory_size_mb;
	uint32_t block_count;
	uint32_t block_size;
 
    CFileIoWrapper();

  

    int acquire_mutex(void);

    void release_mutex(void);

public:

    static int mount();

    static int unmount();

    static CFileIoWrapper *GetInstance();

    static char buffer[CHUNK_SIZE];

    void mountUnmount(void);

    void fs_open_write(const char *file_name,const void *buffer,size_t size,fs_mode_t mode);

    int fs_open_read(const char *file_name,void *buffer,fs_mode_t mode);

};