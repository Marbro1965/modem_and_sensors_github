#pragma once

#include "BaseThread/CBaseThread.h"

class CWriteOnSdCard : public CBaseThread
{

    int copyFirmwareFromSdToNor();

    int address ;

    char buffer[4096];

    char compare_buffer[4096];

public:

    CWriteOnSdCard();
    
    ~CWriteOnSdCard();

    void runHandler();



};