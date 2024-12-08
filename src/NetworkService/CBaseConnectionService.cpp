#include "CBaseConnectionService.h"

#include <zephyr/kernel.h> 

#include "structures.h"

CBaseConnectionService::CBaseConnectionService()
{

}

CBaseConnectionService::~CBaseConnectionService()
{

}

// callback di connessione alla rete LTE
// rilascia il semaforo
void CBaseConnectionService::connectToWiFi(void){

    k_sem_give(&CBaseThread::net_conn_sem);

}

void CBaseConnectionService::disconnectFromWiFi(void){

    struct my_msg msg;

    msg.data = WIFI_DISCONNECT;

    int ret = k_msgq_put(&CBaseThread::registerQueueMessage, &msg, K_NO_WAIT);

}


