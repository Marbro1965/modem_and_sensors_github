#pragma once

#include "BaseThread/CBaseThread.h"


class CDateTimeThread : public CBaseThread
{
    void init(void);
public:
    CDateTimeThread();

    ~CDateTimeThread();

    void runHandler(void) override;
};