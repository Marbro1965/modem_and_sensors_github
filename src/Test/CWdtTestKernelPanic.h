#pragma once

#include "BaseThread/CBaseThread.h"

#include <zephyr/kernel.h>

#include <zephyr/device.h>

#include <zephyr/drivers/gpio.h>

class CWdtTestKernelPanic:public CBaseThread
{

    static const struct device *gpio_dev;

    static CWdtTestKernelPanic* instance;

    static struct gpio_dt_spec button;

    static struct gpio_callback gpio_cb;

    
    bool init(void);

    static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

    void crash_function(uint32_t *addr);


public:
    CWdtTestKernelPanic();  

    virtual ~CWdtTestKernelPanic();

    void runHandler(void) override;

    

};