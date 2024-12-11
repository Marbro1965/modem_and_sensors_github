#ifndef CBASETHREAD_H
#define CBASETHREAD_H

#include <zephyr/kernel.h> // or the appropriate header file that defines k_msgq

#include "Logger/CLogger.h"

#define DEFAULT_THREAD_STACK_SIZE 1024

#define LTE_CONNECTED_FLAG      0x01        // Flag indicating LTE connection

#define LTE_DISCONNECTED_FLAG   0x00        // Flag indicating LTE connection

class CBaseThread
{


public:

    static k_msgq blinkQueueMessage;

    static k_msgq sensorQueueMessage;

    static k_msgq registerQueueMessage;

    static k_msgq loggerQueueMessage;

    static k_msgq firmwareUpdateQueueMessage;

    static k_msgq configurationQueueMessage;

    static k_sem net_conn_sem;

    static k_event lte_event_flags;

    static uint32_t utc_time;

    CBaseThread();

    virtual ~CBaseThread();

    static void handlerRun(void *args1, void *args2, void *args3);

    virtual void runHandler(void) = 0;

    static void convertToReadableTime(uint32_t time);

    static void convertToReadableTime(uint32_t time, char *buffer);
    
};
#endif // CBASETHREAD_H