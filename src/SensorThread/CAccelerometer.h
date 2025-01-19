#pragma once

#include <zephyr/kernel.h>

#include <zephyr/device.h>

#include <zephyr/drivers/sensor.h>

class sensor_trigger;

class sensor_value;

class CAccelerometer
{

    static CAccelerometer *instance;

    struct sensor_trigger *trig;

    struct sensor_value *attr;
    

    CAccelerometer();

    static void trigger_handler(const struct device *dev, const struct sensor_trigger *trig);


public:

    static CAccelerometer *getInstance(){

        if (instance == nullptr)
        {
            instance = new CAccelerometer();
        }

        return instance;
    }  

    void init();

};