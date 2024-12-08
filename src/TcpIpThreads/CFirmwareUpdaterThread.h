#pragma once

#include "BaseThread/CBaseThread.h"

class CFirmwareUpdaterThread : public CBaseThread
{


public:

    CFirmwareUpdaterThread();

    virtual ~CFirmwareUpdaterThread();

    void runHandler(void) override;

    


};