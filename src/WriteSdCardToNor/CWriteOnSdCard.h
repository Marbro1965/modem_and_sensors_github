#pragma once

#include "BaseThread/CBaseThread.h"

#include <zephyr/storage/disk_access.h>

#include <zephyr/fs/fs.h>

#include <ff.h>



class CWriteOnSdCard : public CBaseThread
{

    int copyFirmwareFromSdToNor();

    void dump();


    int readMagicNumber();

    int writeMagicNumberToNor();

    int processCommand();

    int address ;

    int finalAddress;

    static char magicBuffer[16];

    static char buffer[4096];

    static char compare_buffer[4096];

    struct fs_file_t file;

public:

    CWriteOnSdCard();
    
    ~CWriteOnSdCard();

    void runHandler();



};