#include "CFileIoWrapper.h"

#include "Logger/CLogger.h"

#include "BaseThread/CBaseThread.h"

CFileIoWrapper* CFileIoWrapper::m_instance = nullptr;

FATFS CFileIoWrapper::fat_fs = {};

fs_mount_t CFileIoWrapper::mp = {};

char CFileIoWrapper::buffer[1024] = {};

CFileIoWrapper::CFileIoWrapper()
{
    

    /* mounting info */
    mp.type = FS_FATFS;

	mp.fs_data = &fat_fs;

}

int CFileIoWrapper::acquire_mutex(void)
{
    int ret;

    ret = k_mutex_lock(&CBaseThread::fileIoMutex, K_FOREVER); // Wait indefinitely
    if (ret == 0) {
        CLogger::getInstance()->log("Mutex acquired");
    } else {
        CLogger::getInstance()->log("Failed to acquire mutex");
    }

    return ret;
}

void CFileIoWrapper::release_mutex(void)
{
    k_mutex_unlock(&CBaseThread::fileIoMutex);

    CLogger::getInstance()->log("Mutex released");
}

CFileIoWrapper* CFileIoWrapper::GetInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new CFileIoWrapper();
    }

    return m_instance;
}

void CFileIoWrapper::mountUnmount(void)
{

    const char *disk_mount_pt = "/SD:";

    mp.mnt_point = disk_mount_pt;

    int res = fs_mount(&mp);

    if (res == FR_OK) {
        CLogger::getInstance()->log("Disk mounted.\n");
    } else {
        CLogger::getInstance()->log("Error mounting disk.\n");
    }

    res = fs_unmount(&mp);


}

int CFileIoWrapper::mount()
{

    const char *disk_mount_pt = "/SD:";

    mp.mnt_point = disk_mount_pt;

    int res = fs_mount(&mp);

    if (res == FR_OK) {
        CLogger::getInstance()->log("Disk mounted.\n");
    } else {
        CLogger::getInstance()->log("Error mounting disk.\n");
    }

    return res;
}

int CFileIoWrapper::unmount()
{
    int res = fs_unmount(&mp);

    if (res == FR_OK) {
        CLogger::getInstance()->log("Disk unmounted.\n");
    } else {
        CLogger::getInstance()->log("Error unmounting disk.\n");
    }

    return res;
}

void CFileIoWrapper::fs_open_write(const char *file_name,const void *buffer,size_t size,fs_mode_t mode)
{
    acquire_mutex();

    int res = mount();
    if (res!=FR_OK) {
        CLogger::getInstance()->log("Mounting filesystem failed, error: %d", res);
        res = unmount();
        return;
    }

    fs_file_t_init(&file);
    res = fs_open(&file, file_name, mode);

    if (res == FR_OK) {
        CLogger::getInstance()->log("File opened.\n");
    } else {
        CLogger::getInstance()->log("Error opening file.\n");
    }

    ssize_t bytes_written = fs_write(&file, buffer, size);
    if (bytes_written < 0) {
        CLogger::getInstance()->log("Failed to write to file, error: %d", bytes_written);
        fs_close(&file);
        res = unmount();
        release_mutex();
        return;
    }

    CLogger::getInstance()->log("Wrote %d bytes to file", bytes_written);

    fs_close(&file);

    res = unmount();

    release_mutex();
}

int CFileIoWrapper::fs_open_read(const char *file_name,void *buffer,fs_mode_t mode){

    acquire_mutex();

    int res = mount();
    if (res!=FR_OK) {
        CLogger::getInstance()->log("Mounting filesystem failed, error: %d", res);
        res = unmount();
        return -1;
    }

    fs_file_t_init(&file);
    res = fs_open(&file, file_name, mode);

    if (res == FR_OK) {
        CLogger::getInstance()->log("File opened.\n");
    } else {
        CLogger::getInstance()->log("Error opening file.\n");
    }

    ssize_t bytes_read = fs_read(&file, buffer, CHUNK_SIZE);
    if (bytes_read < 0) {
        CLogger::getInstance()->log("Failed to read from file, error: %d", bytes_read);
        fs_close(&file);
        res = unmount();
        return -1;
    }

    fs_close(&file);

    res = unmount();

    release_mutex();

    return bytes_read;
}