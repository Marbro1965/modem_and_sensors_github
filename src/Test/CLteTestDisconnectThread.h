#pragma once

#include "BaseThread/CBaseThread.h"

class CTestDisconnectThread : public CBaseThread
{

    static const struct device *gpio_dev;
    
public:
    CTestDisconnectThread();
    virtual ~CTestDisconnectThread();

    void runHandler(void) override;    
};