#ifndef CBASECONNECTIONSERVICE_H
#define CBASECONNECTIONSERVICE_H

#include "BaseThread/CBaseThread.h"

class CBaseConnectionService:public CBaseThread
{

protected:

public:

    CBaseConnectionService();

    virtual ~CBaseConnectionService();
  

    virtual void connectToWiFi(void);

    virtual void disconnectFromWiFi(void);


};

#endif // CBASECONNECTIONSERVICE_H