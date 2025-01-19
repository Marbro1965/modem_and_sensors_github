#include "CAccelerometer.h"

#include "Logger/CLogger.h"

CAccelerometer *CAccelerometer::instance = nullptr;



CAccelerometer::CAccelerometer(){

    trig = new sensor_trigger;

    attr = new sensor_value;


}

void CAccelerometer::init(){


    const struct device *sensor = DEVICE_DT_GET_ANY(st_lis2dh);

    if (!device_is_ready(sensor)) {
        CLogger::getInstance()->log("Device %s is not ready\n", sensor->name);
        return;
    }


    // Set the threshold (adjust this value as needed)
    attr->val1 = 0;
    attr->val2 = (int32_t)(SENSOR_G * 1.3); // Set to 1.3G

    int rc = sensor_attr_set(sensor, SENSOR_CHAN_ACCEL_XYZ,
                             SENSOR_ATTR_SLOPE_TH, attr);
    if (rc < 0) {
        CLogger::getInstance()->log("Cannot set slope threshold.\n");
        return;
    }

    // Set up the trigger
    trig->type = SENSOR_TRIG_DELTA;
    trig->chan = SENSOR_CHAN_ACCEL_XYZ;

    rc = sensor_trigger_set(sensor, trig, trigger_handler);
    if (rc != 0) {
        CLogger::getInstance()->log("Failed to set trigger: %d\n", rc);
        return;
    }

}


void CAccelerometer::trigger_handler(const struct device *dev, const struct sensor_trigger *trig){


    CLogger::getInstance()->log("Acceleration over threshold\n");

    // Re-enable the trigger to ensure continuous interrupts

    int rc = sensor_trigger_set(dev, trig, trigger_handler);
    if (rc != 0) {
        CLogger::getInstance()->log("Failed to re-enable trigger: %d\n", rc);
    }

}