#include "WdtThread/CWdtThread.h"


CWdtThread::CWdtThread()
{
}

CWdtThread::~CWdtThread()
{
}

void CWdtThread::init()
{

    wdt_dev = DEVICE_DT_GET(DT_NODELABEL(wdt));

    if (!wdt_dev) {
        //printk("Watchdog device not found\n");
        //return;
    }

    // Watchdog timeout configuration
    wdt_cfg.window.max = (uint32_t)5000;
    wdt_cfg.window.min = 0;
    wdt_cfg.callback = NULL;
    wdt_cfg.flags = WDT_FLAG_RESET_SOC;

    // Install timeout configuration for the watchdog
    wdt_id = wdt_install_timeout(wdt_dev, &wdt_cfg);
    if (wdt_id < 0) {
        //printk("Failed to install watchdog timeout\n");
        //return;
    }

    // Start the watchdog timer
    wdt_setup(wdt_dev, 0);
    
}


void CWdtThread::runHandler(void)
{
    // Check if the watchdog timer has expired
    // If the watchdog timer has expired, reset the system
    // If the watchdog timer has not expired, continue normal operation
    init();
    
    while(true)
    {
        // Check if the watchdog timer has expired
        // If the watchdog timer has expired, reset the system
        // If the watchdog timer has not expired, continue normal operation
        wdt_feed(wdt_dev, wdt_id);
        k_sleep(K_MSEC(1000));
    }


}


