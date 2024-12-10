#pragma once

#include "BaseThread/CBaseThread.h"

class CUtcTimeThread : public CBaseThread
{
public:
    CUtcTimeThread();
    virtual ~CUtcTimeThread();

    void runHandler(void) override;
    
};