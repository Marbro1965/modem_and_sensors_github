#include "CWriteOnSdCard.h"

#include <zephyr/kernel.h>

#include <zephyr/storage/disk_access.h>

#include <zephyr/fs/fs.h>

#include <ff.h>

#include "SdCard/CFileIoWrapper.h"

#include "NorHelper/CNorHelper.h"

char CWriteOnSdCard::buffer[4096]={};


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
            copyFirmwareFromSdToNor();

        }

        k_sleep(K_MSEC(1000));


    }
 
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

    // res = CFileIoWrapper::mount();

    // fs_file_t_init(&file);

    // res = fs_open(&file, "/SD:/dump.bin", FS_O_CREATE | FS_O_WRITE);

    // if (res == FR_OK) {
    //     CLogger::getInstance()->log("File opened.\n");
    // } else {
    //     CLogger::getInstance()->log("Error opening file.\n");
    // }
    // address = 0x0000;

    // while (address < finalAddress) {
    //     size_t bytesToRead = (address + 4096 <= finalAddress) ? 4096 : (finalAddress - address);

    //     // Read a chunk of data from the NOR flash
    //     if (CNorHelper::readDataFromNor(address, buffer, bytesToRead) == 0) {
            
    //         //write to file
    //         res = fs_write(&file, buffer, bytesToRead);
    //     }
    //     // Move to the next chunk
    //     address += res;
    // }

    // fs_close(&file);

    // res = CFileIoWrapper::unmount();

    CLogger::getInstance()->log("Firmware copied from SD to NOR\n");

    //boot_set_pending_multi(0, 1);

    return 0;

}