#include "CLteTestDisconnectThread.h"

#include <modem/lte_lc.h>

CTestDisconnectThread::CTestDisconnectThread()
{
}

CTestDisconnectThread::~CTestDisconnectThread()
{
    
}

void CTestDisconnectThread::runHandler(void)
{
    int counter = 0;

    while(true) {

        uint32_t events = k_event_wait(&CBaseThread::lte_event_flags, LTE_CONNECTED_FLAG, false, K_FOREVER);
        if (events & LTE_CONNECTED_FLAG) {
            
            counter++;        

        }

        if (counter>60){

            lte_lc_offline();

            counter = 0;
        }

        k_sleep(K_SECONDS(1));
    
    
    
    }
}