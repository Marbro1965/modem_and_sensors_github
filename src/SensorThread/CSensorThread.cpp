#include "CSensorThread.h"

#include <zephyr/kernel.h>

#include "structures.h"

#include "Logger/CLogger.h"

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
	
	while (true) {
		//struct sensor_value temp, press, humidity, iaq, co2, voc;
		struct messageSensor msg;
		sensor_sample_fetch(dev);
		sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &msg.temp);
		sensor_channel_get(dev, SENSOR_CHAN_PRESS, &msg.press);
		sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &msg.humidity);
		sensor_channel_get(dev, (enum sensor_channel)SENSOR_CHAN_IAQ, &msg.iaq);
		sensor_channel_get(dev, SENSOR_CHAN_CO2, &msg.co2);
		sensor_channel_get(dev, SENSOR_CHAN_VOC, &msg.voc);

		CLogger::getInstance()->log("temp: %d.%06d; press: %d.%06d; humidity: %d.%06d; iaq: %d; CO2: %d.%06d; "
		 	"VOC: %d.%06d \n",
		 	msg.temp.val1, msg.temp.val2, msg.press.val1, msg.press.val2,
			msg.humidity.val1, msg.humidity.val2,
		 	msg.iaq.val1, msg.co2.val1, msg.co2.val2, msg.voc.val1, msg.voc.val2);

		int ret = k_msgq_put(&CBaseThread::sensorQueueMessage, &msg, K_NO_WAIT);

		k_sleep(K_MSEC(5000));
	}


}