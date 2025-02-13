#ifndef CMODEMSETUPTHREAD_H
#define CMODEMSETUPTHREAD_H

#include "NetworkService\CBaseConnectionService.h"

//#include <nrf_modem.h>

#include <nrf_modem_lib.h>

#include <nrf_modem_at.h>

#include <modem/lte_lc.h>

//#include <modem/nrf_modem_lib.h>

class CModemSetupThread : public CBaseConnectionService
{
public:
    enum modem_state
    {
        MODEM_STATE_OFF,
        MODEM_STATE_ON,
        MODEM_STATE_READY,
        MODEM_STATE_ERROR,
        MODEM_STATE_CONNECTED,
        MODEM_STATE_CONNECTING,
        MODEM_STATE_DISCONNECTING,        
        MODEM_STATE_DISCONNECTED        
    };

protected:
    
    void initModemLibrary(void);

    void initModem(void);



    //void startLocation(void);

public:

    modem_state modem_state;
    
    CModemSetupThread();
    
    virtual ~CModemSetupThread();

    void runHandler(void) override;
 
    static void lte_handler(const struct lte_lc_evt *const evt);

    static void configure_psm();

    //static void location_event_handler(const struct location_event_data *event_data);

    static CModemSetupThread *instance;

    void onTimerCallback() override;

};

#endif // CMODEMSETUPTHREAD_H