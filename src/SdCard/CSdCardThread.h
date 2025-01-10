#pragma once

#include <zephyr/storage/disk_access.h>
#include <zephyr/fs/fs.h>


#include "BaseThread/CBaseThread.h"


#include <ff.h>

/*
 *  Note the fatfs library is able to mount only strings inside _VOLUME_STRS
 *  in ffconf.h
 */
#define DISK_DRIVE_NAME "SD"

#define DISK_MOUNT_PT "/"DISK_DRIVE_NAME":"




class CSdCardThread : public CBaseThread
{

    static FATFS fat_fs;

    static fs_mount_t mp;

    uint64_t memory_size_mb;
	uint32_t block_count;
	uint32_t block_size;


public:
        
    CSdCardThread();
    
    ~CSdCardThread();
    
    void runHandler(void) override;

};