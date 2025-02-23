#pragma once

#include "BaseThread/CBaseThread.h"

#include <zephyr/storage/disk_access.h>

#include <zephyr/fs/fs.h>

#include <ff.h>



class CWriteOnSdCard : public CBaseThread
{

    int copyFirmwareFromSdToNor();

    int address ;

    static char buffer[4096];

    static char compare_buffer[4096];

    struct fs_file_t file;

public:

    CWriteOnSdCard();
    
    ~CWriteOnSdCard();

    void runHandler();



};