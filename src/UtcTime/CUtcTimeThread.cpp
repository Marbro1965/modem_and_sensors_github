#include "CUtcTimeThread.h"

#include <nrf_socket.h>

CUtcTimeThread::CUtcTimeThread()
{
}


CUtcTimeThread::~CUtcTimeThread()
{
}

void CUtcTimeThread::runHandler(void)
{

    int sock = nrf_socket(NRF_AF_INET, NRF_SOCK_STREAM, NRF_IPPROTO_TCP);
    if (sock < 0) {
        //printk("Failed to create socket\n");
        return;
    }
    while (1)
    {
        k_sleep(K_SECONDS(1));
    }
}