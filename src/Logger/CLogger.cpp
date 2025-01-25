#include "CLogger.h"

#include "BaseThread/CBaseThread.h"

#include <stdarg.h>

#include <sstream>


CLogger *CLogger::instance = nullptr;


CLogger::CLogger()
{
    
}

void CLogger::log(const char *format, ...)
{
    va_list args;

    va_start(args, format);

    // Use vsnprintf to avoid buffer overflow
    vsnprintf(&pt[0], sizeof(pt), format, args);

    va_end(args);

    int res = k_msgq_put(&CBaseThread::loggerQueueMessage, &pt[0], K_NO_WAIT);


}

