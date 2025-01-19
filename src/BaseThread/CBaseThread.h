#ifndef CBASETHREAD_H
#define CBASETHREAD_H

#include <zephyr/kernel.h> // or the appropriate header file that defines k_msgq

#include <vector>

#include "Logger/CLogger.h"

#include "structures.h"

#define DEFAULT_THREAD_STACK_SIZE 1024

#define LTE_CONNECTED_FLAG      0x01        // Flag indicating LTE connection

#define LTE_DISCONNECTED_FLAG   0x00        // Flag indicating LTE connection

#define MQTT_MESSAGE_TO_SEND_FLAG 0x01

class CBaseThread
{

protected:

    struct k_timer my_timer;

    void createTimer();

    void startOneShotTimer(uint32_t duration_ms);

    void stopOneShotTimer();


public:

    static k_msgq blinkQueueMessage;

    static k_msgq sensorQueueMessage;

    static k_msgq registerQueueMessage;

    static k_msgq loggerQueueMessage;

    static k_msgq msgDownloadClient;

    static k_msgq msgAckClient;

    static k_sem net_conn_sem;

    static k_event lte_event_flags;

    static k_event mqttMessageInQueueFlag;

    static int64_t unix_time_ms;

    CBaseThread();

    virtual ~CBaseThread();

    static void handlerRun(void *args1, void *args2, void *args3);

    virtual void runHandler(void) = 0;

    static void convertToReadableTime(uint32_t time);

    static void convertToReadableTime(uint32_t time, char *buffer);

    static void read_serial_number();

    static void timerCallback(struct k_timer *timer_id);

    virtual void onTimerCallback();
    
};
#endif // CBASETHREAD_H