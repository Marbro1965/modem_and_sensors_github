#include "CSensorThread.h"

#include <zephyr/kernel.h>



#include "Logger/CLogger.h"

#include "CAccelerometer.h"

CSensorThread::CSensorThread()
{

	


}

CSensorThread::~CSensorThread()
{
}

int CSensorThread::init(void)
{
    // This is the init function for the sensor thread

   	dev = DEVICE_DT_GET_ANY(bosch_bme680);

	//LOG_INF("App started");

	//k_sleep(K_SECONDS(5));

	if (dev == NULL) {
		//LOG_ERR("no device found");
		return 0;
	}
	if (!device_is_ready(dev)) {
		//LOG_ERR("device is not ready");
		return 0;
	}

    return 1;

}

void CSensorThread::runHandler(void)
{
    // This is the handler for the sensor thread
	CLogger::getInstance()->log("Esecuzione del thread CSensorThread\n");
	
	if (1==init())
	{
		//LOG_INF("Sensor thread initialized");
	}
	else
	{
		//LOG_ERR("Sensor thread failed to initialize");
	}	

	CAccelerometer::getInstance()->init();
	
	while (true) {
		//struct sensor_value temp, press, humidity, iaq, co2, voc;
		struct messageSensor msg;

		fetchSensor(msg);

		int ret = k_msgq_put(&CBaseThread::sensorQueueMessage, &msg, K_NO_WAIT);

		k_sleep(K_MSEC(5000));

	}


}


void CSensorThread::fetchSensor(messageSensor &msg){


		sensor_sample_fetch(dev);
		sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &msg.temp);
		sensor_channel_get(dev, SENSOR_CHAN_PRESS, &msg.press);
		sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &msg.humidity);
		sensor_channel_get(dev, (enum sensor_channel)SENSOR_CHAN_IAQ, &msg.iaq);
		sensor_channel_get(dev, SENSOR_CHAN_CO2, &msg.co2);
		sensor_channel_get(dev, SENSOR_CHAN_VOC, &msg.voc);

}