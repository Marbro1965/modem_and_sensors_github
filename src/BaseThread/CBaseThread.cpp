#include "CBaseThread.h"
                       // Include the header file that defines k_mem_pool


k_msgq CBaseThread::blinkQueueMessage{};    // Initialize the static member variable

k_msgq CBaseThread::registerQueueMessage{}; // Initialize the static member variable

k_msgq CBaseThread::sensorQueueMessage{};

k_msgq CBaseThread::loggerQueueMessage{};

k_msgq CBaseThread::firmwareUpdateQueueMessage{};

k_msgq CBaseThread::configurationQueueMessage{};

k_event CBaseThread::lte_event_flags{};


k_sem CBaseThread::net_conn_sem{};          // Initialize the static member variable



CBaseThread::CBaseThread()
{
}

CBaseThread::~CBaseThread()
{
}

void CBaseThread::handlerRun(void *args1, void *args2, void *args3)
{
    CBaseThread *pThread = (CBaseThread *)args1;
    pThread->runHandler();
}   
