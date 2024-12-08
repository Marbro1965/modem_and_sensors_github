#pragma once

#include "BaseThread/CBaseThread.h"

class CConfigurationReceiverThread : public CBaseThread
{

public:
    
        CConfigurationReceiverThread();

        virtual ~CConfigurationReceiverThread();

        void runHandler(void) override;

};