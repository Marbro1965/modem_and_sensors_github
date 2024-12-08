#pragma once

#include "BaseThread/CBaseThread.h"

#include <zephyr/logging/log.h>


class CLoggerThread : public CBaseThread
{

    int init(void);

    char pt[128];

public:
        
    CLoggerThread();
    
    ~CLoggerThread();
    
    void runHandler(void) override;
};