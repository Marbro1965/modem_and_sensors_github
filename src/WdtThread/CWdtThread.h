#pragma once

#include "BaseThread/CBaseThread.h"

#include <zephyr/kernel.h>

#include <zephyr/device.h>

#include <zephyr/drivers/watchdog.h>

class CWdtThread : public CBaseThread
{

    const struct device *wdt_dev;

    struct wdt_timeout_cfg wdt_cfg;

    int wdt_id;

    void init();

public:
    CWdtThread();

    virtual ~CWdtThread();


    void runHandler(void) override;



};