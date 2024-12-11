#include "CBaseThread.h"

#include "structures.h"

#include <time.h>
                       // Include the header file that defines k_mem_pool


k_msgq CBaseThread::blinkQueueMessage{};    // Initialize the static member variable

k_msgq CBaseThread::registerQueueMessage{}; // Initialize the static member variable

k_msgq CBaseThread::sensorQueueMessage{};

k_msgq CBaseThread::loggerQueueMessage{};

k_msgq CBaseThread::firmwareUpdateQueueMessage{};

k_msgq CBaseThread::configurationQueueMessage{};

k_event CBaseThread::lte_event_flags{};


k_sem CBaseThread::net_conn_sem{};          // Initialize the static member variable

uint32_t CBaseThread::utc_time{};

std::vector<k_msgq> CBaseThread::registeredQueue{};

CBaseThread::CBaseThread()
{
}

CBaseThread::~CBaseThread()
{

}


void CBaseThread::registerThread(void)
{
    
    k_msgq_init(&disconnectedQueueMessage,&my_msgq_registered_thread[0], sizeof(struct my_msg), 10);   
    
    registeredQueue.push_back(disconnectedQueueMessage);
    
}


void CBaseThread::handlerRun(void *args1, void *args2, void *args3)
{
    CBaseThread *pThread = (CBaseThread *)args1;
    pThread->runHandler();
}   

void CBaseThread::convertToReadableTime(uint32_t seconds_since_1970)
{
    // Log the UTC time
    time_t raw_time = seconds_since_1970;
    struct tm *time_info = gmtime(&raw_time);

    // Format the date and time as a string
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

    // Log the UTC time
    CLogger::getInstance()->log("UTC time: %s\n", time_str);

}

void CBaseThread::convertToReadableTime(uint32_t seconds_since_1970, char *buffer)
{

   // Log the UTC time
    time_t raw_time = seconds_since_1970;
    struct tm *time_info = gmtime(&raw_time);

    // Format the date and time as a string
    
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time_info);

    // Log the UTC time
    CLogger::getInstance()->log("UTC time: %s\n", buffer);

}