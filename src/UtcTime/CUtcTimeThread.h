#pragma once

#include "BaseThread/CBaseThread.h"

class CUtcTimeThread : public CBaseThread
{

    int getUtcTime(void);
public:
    CUtcTimeThread();
    virtual ~CUtcTimeThread();

    void runHandler(void) override;
    
};