#include "CDateTimeThread.h"
#include <date_time.h>

CDateTimeThread::CDateTimeThread()
{
}

CDateTimeThread::~CDateTimeThread()
{
}

void CDateTimeThread::init(void)
{

    int err;
    

    // Initialize and enable the Date Time library
    date_time_register_handler(NULL);

    err = date_time_update_async(NULL);

    if (err) {
        CLogger::getInstance()->log("Failed to initialize Date Time library, error: %d\n", err);
        return;
    }

}
void CDateTimeThread::runHandler(void)
{

    //int64_t unix_time_ms;
    
    CLogger::getInstance()->log("DateTime Thread started\n");

    init();

    // Wait for time to be synchronized
    while (!date_time_is_valid()) {
        k_sleep(K_SECONDS(1));
    }

    while (true)
    {

        int err = date_time_now(&CBaseThread::unix_time_ms);
        // Sleep for a while before checking again
        k_sleep(K_SECONDS(300)); // Check every hour to synchronize
    }
}


