#include "CWriteOnSdCard.h"

#include <zephyr/kernel.h>

#include <zephyr/storage/disk_access.h>

#include <zephyr/fs/fs.h>

#include <ff.h>

#include "SdCard/CFileIoWrapper.h"

#include "NorHelper/CNorHelper.h"

char CWriteOnSdCard::buffer[4096]={};

char CWriteOnSdCard::magicBuffer[16]={0x77, 0xC2, 0x95, 0xF3, 0x60, 0xD2, 0xEF, 0x7F, 0x35, 0x52, 0x50, 0x0F, 0x2C, 0xB6, 0x79, 0x80};


char CWriteOnSdCard::compare_buffer[4096]={};



CWriteOnSdCard::CWriteOnSdCard()
{
    
}

CWriteOnSdCard::~CWriteOnSdCard()
{
    
}



void CWriteOnSdCard::runHandler()
{

    my_msg msg;

    while(true){

        k_msgq_get(&CBaseThread::copySdToNor, &msg, K_FOREVER);

        if (msg.data == COPY_FROM_SD_TO_NOR)
        {
            CLogger::getInstance()->log("Copy from SD to NOR\n");

            // Copy the firmware from the SD card to the NOR memory
            processCommand();

        }

        k_sleep(K_MSEC(1000));


    }
 
}

int CWriteOnSdCard::processCommand()
{
    
    int ret = k_mutex_lock(&CBaseThread::firmwareUpgradeMutex, K_FOREVER);

    CNorHelper::eraseAllMemorySecondarySlot();

    //readMagicNumber();

    copyFirmwareFromSdToNor();

    writeMagicNumberToNor();

    //dump();

    k_mutex_unlock(&CBaseThread::firmwareUpgradeMutex);

    return ret;
}


int CWriteOnSdCard::readMagicNumber(){

    const char *filename = "/SD:/signed.bin";

    int err = 0;

    
    CLogger::getInstance()->log("Read magic number from signed file\n");

    CFileIoWrapper *fileIoWrapper = CFileIoWrapper::GetInstance();

    int res = CFileIoWrapper::mount();

    if (res!=FR_OK) {
        CLogger::getInstance()->log("Mounting filesystem failed, error: %d", res);
        res = CFileIoWrapper::unmount();
        return -1;
    }

    fs_file_t_init(&file);

    res = fs_open(&file, filename, FS_O_READ);

    if (fs_seek(&file, -16, FS_SEEK_END) != 0) {
        CLogger::getInstance()->log("Failed to seek in file\n");
        fs_close(&file);
        return -1;
    }

    int bytes_read = fs_read(&file, magicBuffer, sizeof(magicBuffer));
    if (bytes_read < 0) {
        CLogger::getInstance()->log("Failed to read file\n");
    } else {
        CLogger::getInstance()->log("Read %d bytes successfully\n", bytes_read);
    }

    // Close the file
    fs_close(&file);

    res = CFileIoWrapper::unmount();

    return 0;
}

int CWriteOnSdCard::writeMagicNumberToNor(){

    int err = CNorHelper::writeDataToNor(0xE7FF0, magicBuffer, compare_buffer, sizeof(magicBuffer));

    return err;
}

int CWriteOnSdCard::copyFirmwareFromSdToNor()
{

    const char *filename = "/SD:/signed.bin";

    int err = 0;

    // Copy the firmware from the SD card to the NOR memory
    CLogger::getInstance()->log("Copying firmware from SD to NOR\n");

    CFileIoWrapper *fileIoWrapper = CFileIoWrapper::GetInstance();

    int res = CFileIoWrapper::mount();

    if (res!=FR_OK) {
        CLogger::getInstance()->log("Mounting filesystem failed, error: %d", res);
        res = CFileIoWrapper::unmount();
        return -1;
    }

    fs_file_t_init(&file);

    res = fs_open(&file, filename, FS_O_READ);

    if (res == FR_OK) {
        CLogger::getInstance()->log("File opened.\n");
    } else {
        CLogger::getInstance()->log("Error opening file.\n");
    }
    address = 0x0000;
    finalAddress = address;
    while (true) {
        memset(buffer, 0xFF, 4096);  // Fill buffer with 0xFF initially
        memset(compare_buffer, 0xFF, 4096);  // Fill buffer with 0xFF initially
        res = fs_read(&file, buffer, 4096);  // Read up to 4096 bytes
        finalAddress += res;
        if (res == 0) {
            CLogger::getInstance()->log("End of file reached\n");
            break;  // EOF
        }
        if (res < 0) {
            CLogger::getInstance()->log("Error reading file, error code: %d\n", res);
            break;  // Error case
        }

        CLogger::getInstance()->log("Read %d bytes from file\n", res);

        // Process buffer here if needed...
        err = CNorHelper::writeDataToNor(address, buffer, compare_buffer, 4096);

        if (err < 0) {
            CLogger::getInstance()->log("Error writing nor, error code: %d\n", err);
            break;  // Error case
        }

        address += 4096;
        // If res < CHUNK_SIZE, the remaining bytes are already 0xFF
    }


    fs_close(&file);

    res = CFileIoWrapper::unmount();

    CLogger::getInstance()->log("Firmware copied from SD to NOR\n");

    return 0;

}

void CWriteOnSdCard::dump(){

    int res = CFileIoWrapper::mount();

    fs_file_t_init(&file);

    res = fs_open(&file, "/SD:/dump.bin", FS_O_CREATE | FS_O_WRITE);

    if (res == FR_OK) {
        CLogger::getInstance()->log("File opened.\n");
    } else {
        CLogger::getInstance()->log("Error opening file.\n");
    }
    address = 0x0000;

    finalAddress = 0xE8000;

    while (address < finalAddress) {
        size_t bytesToRead = (address + 4096 <= finalAddress) ? 4096 : (finalAddress - address);

        // Read a chunk of data from the NOR flash
        if (CNorHelper::readDataFromNor(address, buffer, bytesToRead) == 0) {
            
            //write to file
            res = fs_write(&file, buffer, bytesToRead);
        }
        // Move to the next chunk
        address += res;
    }

    fs_close(&file);

    res = CFileIoWrapper::unmount();

    CLogger::getInstance()->log("Firmware copied from SD to NOR\n");

    //boot_set_pending_multi(0, 1);



}