#include "CSdCardThread.h"

FATFS CSdCardThread::fat_fs = {};

fs_mount_t CSdCardThread::mp = {};

CSdCardThread::CSdCardThread()
{

    /* mounting info */
    mp.type = FS_FATFS;

	mp.fs_data = &fat_fs;

}

CSdCardThread::~CSdCardThread()
{
}

void CSdCardThread::runHandler(void)
{


    CLogger::getInstance()->log("Esecuzione del thread CSdCardThread\n");

    // const char *disk_pdrv = DISK_DRIVE_NAME;

    // if (disk_access_init(disk_pdrv) != 0) {
			
	//     CLogger::getInstance()->log("Storage init ERROR\n");
    // }

    // if (disk_access_ioctl(disk_pdrv,
	// 			DISK_IOCTL_GET_SECTOR_COUNT, &block_count)) {
	// 	CLogger::getInstance()->log("Unable to get sector count");
		
	// }

	// CLogger::getInstance()->log("Block count %u", block_count);

	// if (disk_access_ioctl(disk_pdrv,
	// 			DISK_IOCTL_GET_SECTOR_SIZE, &block_size)) {
	// 	CLogger::getInstance()->log("Unable to get sector size");
		
	// }

	// CLogger::getInstance()->log("Sector size %u\n", block_size);

	// memory_size_mb = (uint64_t)block_count * block_size;

	// CLogger::getInstance()->log("Memory Size(MB) %u\n", (uint32_t)(memory_size_mb >> 20));

    const char *disk_mount_pt = "/SD:";

    mp.mnt_point = disk_mount_pt;

    int res = fs_mount(&mp);

    if (res == FR_OK) {
        CLogger::getInstance()->log("Disk mounted.\n");
    } else {
        CLogger::getInstance()->log("Error mounting disk.\n");
    }

    res = fs_unmount(&mp);

    while(true)
    {

        k_sleep(K_SECONDS(1));
    }
}