#include "CLoggerThread.h"

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>

//LOG_MODULE_REGISTER(clogger_thread, LOG_LEVEL_INF);


CLoggerThread::CLoggerThread()
{
 
}

CLoggerThread::~CLoggerThread()
{
 
}

int CLoggerThread::init(void)
{
//    LOG_INF("App started");

//    printk("Hello, World!\n");

    return 1;
}
void CLoggerThread::runHandler(void)
{
    int ret = 0;

    init();

    while(1)
    {

        ret = k_msgq_get(&CBaseThread::loggerQueueMessage, &pt[0], K_NO_WAIT);
        
        if (ret == 0)
        {
           printk("Logger: %s\n",pt);
        }

        k_sleep(K_MSEC(10));
    }
}

