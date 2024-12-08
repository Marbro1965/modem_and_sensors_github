#include "CFirmwareUpdaterThread.h"

CFirmwareUpdaterThread::CFirmwareUpdaterThread()
{
    //ctor
}


CFirmwareUpdaterThread::~CFirmwareUpdaterThread()
{
    //dtor
}

void CFirmwareUpdaterThread::runHandler(void)
{
    while(true)
    {
        k_sleep(K_SECONDS(1));
    }
}

