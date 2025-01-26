#pragma once

#include "BaseThread/CBaseThread.h"

#include "structures.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <stdio.h>
#include <zephyr/sys/__assert.h>
#include <drivers/bme68x_iaq.h>

#include <zephyr/logging/log.h>

class CSensorThread : public CBaseThread
{
    const struct device *dev {};

    int init(void);

    void fetchSensor(messageSensor &msg);
public:
    
    CSensorThread();
    ~CSensorThread();
    
    void runHandler(void) override;
            


};