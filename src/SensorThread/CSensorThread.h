#pragma once

#include "BaseThread/CBaseThread.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
//#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <zephyr/sys/__assert.h>
#include <drivers/bme68x_iaq.h>

#include <zephyr/logging/log.h>

//LOG_MODULE_REGISTER(app, CONFIG_APP_LOG_LEVEL);

class CSensorThread : public CBaseThread
{
    const struct device *dev {};

    int init(void);
public:
    
    CSensorThread();
    ~CSensorThread();
    
    void runHandler(void) override;
            


};