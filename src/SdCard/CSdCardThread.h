#pragma once


#include "BaseThread/CBaseThread.h"

/*
 *  Note the fatfs library is able to mount only strings inside _VOLUME_STRS
 *  in ffconf.h
 */

class CSdCardThread : public CBaseThread
{

public:
        
    CSdCardThread();
    
    ~CSdCardThread();
    
    void runHandler(void) override;

};