#include "initializer.h"

#include "SensorThread/CSensorThread.h"

#include "LoggerThread/CLoggerThread.h"

#include "NetworkService/CModemSetupThread.h"

#include "MqttHelper/CMqttHelperThread.h"

#include "UtcTime/CUtcTimeThread.h"

#include "Leds/CLedsThread.h"

#include "DateTimeThread/CDateTimeThread.h"

#include "WdtThread/CWdtThread.h"

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

    k_event_init(&CBaseThread::lte_event_flags);
    
}

K_THREAD_STACK_DEFINE(thread_logger_stack, DEFAULT_THREAD_STACK_SIZE);
 
K_THREAD_STACK_DEFINE(thread_leds_stack, DEFAULT_THREAD_STACK_SIZE);

K_THREAD_STACK_DEFINE(thread_sensor_stack, DEFAULT_THREAD_STACK_SIZE);

K_THREAD_STACK_DEFINE(thread_modem_stack, 4096);

K_THREAD_STACK_DEFINE(thread_mqtt_stack, DEFAULT_THREAD_STACK_SIZE);

K_THREAD_STACK_DEFINE(thread_utc_time, DEFAULT_THREAD_STACK_SIZE);

K_THREAD_STACK_DEFINE(thread_date_time, DEFAULT_THREAD_STACK_SIZE);

K_THREAD_STACK_DEFINE(thread_wdt, DEFAULT_THREAD_STACK_SIZE);


struct k_thread thread_logger_data;

struct k_thread thread_leds_data;

struct k_thread thread_sensor_data;

struct k_thread thread_modem_data;

struct k_thread thread_mqtt_data;

struct k_thread thread_utc_time_data;

struct k_thread thread_date_time_data;

struct k_thread thread_wdt_data;


void initialize(void){

    initMessageQueue();

    initSemaphore();    

    CLogger::getInstance()->log("Creazione dei threads\n");


    CLoggerThread *pLoggerThread = new CLoggerThread();
 
    k_tid_t id1 = k_thread_create(&thread_logger_data,thread_logger_stack, DEFAULT_THREAD_STACK_SIZE, &CBaseThread::handlerRun, pLoggerThread, NULL, NULL, 10, 0, K_NO_WAIT);


    CLedsThread *pLedsThread = new CLedsThread();

    k_tid_t id2 = k_thread_create(&thread_leds_data,thread_leds_stack, DEFAULT_THREAD_STACK_SIZE, &CBaseThread::handlerRun, pLedsThread, NULL, NULL, 6, 0, K_NO_WAIT);
    
 
    CSensorThread *pSensorThread = new CSensorThread();

    k_tid_t id3 = k_thread_create(&thread_sensor_data,thread_sensor_stack, DEFAULT_THREAD_STACK_SIZE, &CBaseThread::handlerRun, pSensorThread, NULL, NULL, 7, 0, K_NO_WAIT);


    CModemSetupThread *pModemThread = new CModemSetupThread();

    k_tid_t id4 = k_thread_create(&thread_modem_data,thread_modem_stack, 4096, &CBaseThread::handlerRun, pModemThread, NULL, NULL, 1, 0, K_NO_WAIT);


   CMqttHelperThread *pMqttThread = new CMqttHelperThread();   

   k_tid_t id5 = k_thread_create(&thread_mqtt_data,thread_mqtt_stack, 4096, &CBaseThread::handlerRun, pMqttThread, NULL, NULL, 1, 0, K_NO_WAIT);


    CDateTimeThread *pDateTimeThread = new CDateTimeThread();

    k_tid_t id7 = k_thread_create(&thread_date_time_data,thread_date_time, DEFAULT_THREAD_STACK_SIZE, &CBaseThread::handlerRun, pDateTimeThread, NULL, NULL, 10, 0, K_NO_WAIT);


    CUtcTimeThread *pUtcTimeThread = new CUtcTimeThread();  

    k_tid_t id8 = k_thread_create(&thread_utc_time_data,thread_utc_time, DEFAULT_THREAD_STACK_SIZE, &CBaseThread::handlerRun, pUtcTimeThread, NULL, NULL,
    K_PRIO_COOP(1), 0, K_NO_WAIT);

    
}