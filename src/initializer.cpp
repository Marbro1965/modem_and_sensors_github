#include "initializer.h"

#include "SensorThread/CSensorThread.h"

#include "LoggerThread/CLoggerThread.h"

#include "NetworkService/CModemSetupThread.h"

#include "MqttHelper/CMqttHelperThread.h"

#include "structures.h"

#include <zephyr/kernel.h>

#include "Factory/CFactoryThread.h"

#include "Logger/CLogger.h"

char __aligned(4) my_msgq_sensor[MSG_SENSOR_MAX_MSGS * sizeof(struct messageSensor)];
char __aligned(4) my_msgq_buffer[MSGQ_MAX_MSGS * sizeof(struct my_msg)];
char __aligned(4) my_msgq_logger[MSGQ_MAX_MSGS_LOGGER * sizeof(struct logger_msg)];
char __aligned(4) my_msgq_leds[10 * sizeof(struct my_msg)];


void initMessageQueue(void){

    k_msgq_init(&CBaseThread::sensorQueueMessage,&my_msgq_sensor[0], sizeof(struct messageSensor), MSG_SENSOR_MAX_MSGS);

    k_msgq_init(&CBaseThread::loggerQueueMessage,&my_msgq_logger[0], sizeof(struct logger_msg), MSGQ_MAX_MSGS_LOGGER);

    k_msgq_init(&CBaseThread::configurationQueueMessage,&my_msgq_buffer[0], sizeof(struct my_msg), 1);

    k_msgq_init(&CBaseThread::firmwareUpdateQueueMessage,&my_msgq_buffer[0], sizeof(struct my_msg), 1);

    k_msgq_init(&CBaseThread::blinkQueueMessage,&my_msgq_leds[0], sizeof(struct my_msg), 10);



}

void initSemaphore(void){


}

void initialize(void){

    initMessageQueue();

    initSemaphore();    

    CLogger::getInstance()->log("Creazione dei threads\n");

    CFactoryThread::getInstance()->createThreads();



}