#include <stdint.h>

#include <cstdio>

#include "CBaseThread.h"


#include "structures.h"

#include <zephyr/device.h>

#include <zephyr/drivers/hwinfo.h>

#include <time.h>

#include <tfm/tfm_ioctl_api.h>

#include <tfm_ns_interface.h>

#include <hal/nrf_uicr.h>

#include <nrfx.h>


                       // Include the header file that defines k_mem_pool


k_msgq CBaseThread::blinkQueueMessage{};    // Initialize the static member variable

k_msgq CBaseThread::registerQueueMessage{}; // Initialize the static member variable

k_msgq CBaseThread::sensorQueueMessage{};

k_msgq CBaseThread::loggerQueueMessage{};

k_msgq CBaseThread::msgDownloadClient{};

k_msgq CBaseThread::msgAckClient{};

k_msgq CBaseThread::copySdToNor{};

k_event CBaseThread::lte_event_flags{};

k_event CBaseThread::mqttMessageInQueueFlag{};

k_mutex CBaseThread::fileIoMutex{};

k_mutex CBaseThread::firmwareUpgradeMutex{};

k_sem CBaseThread::net_conn_sem{};          // Initialize the static member variable

int64_t CBaseThread::unix_time_ms{};

char CBaseThread::SERIAL_NUMBER[16] = {};

char CBaseThread::RELEASE[16] = {};

CBaseThread::CBaseThread()
{
}

CBaseThread::~CBaseThread()
{

}

void CBaseThread::createTimer()
{

    k_timer_init(&my_timer, &CBaseThread::timerCallback, NULL);

    my_timer.user_data = this;
}

void CBaseThread::handlerRun(void *args1, void *args2, void *args3)
{
    CBaseThread *pThread = (CBaseThread *)args1;
    pThread->runHandler();
}   

void CBaseThread::convertToReadableTime(uint32_t seconds_since_1970)
{
    // Log the UTC time
    time_t raw_time = seconds_since_1970;
    struct tm *time_info = gmtime(&raw_time);

    // Format the date and time as a string
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);

    // Log the UTC time
    CLogger::getInstance()->log("UTC time: %s\n", time_str);

}

void CBaseThread::convertToReadableTime(uint32_t seconds_since_1970, char *buffer)
{

   // Log the UTC time
    time_t raw_time = seconds_since_1970;
    struct tm *time_info = gmtime(&raw_time);

    // Format the date and time as a string
    
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time_info);

    // Log the UTC time
    CLogger::getInstance()->log("UTC time: %s\n", buffer);

}

void CBaseThread::read_serial_number(void)
{
    uint8_t id[8]; // The nRF9160 has an 8-byte unique identifier
    ssize_t length = hwinfo_get_device_id(id, sizeof(id));
    char buffer[17]={};
    int pos = 0;
    if (length > 0) {
        CLogger::getInstance()->log("Device ID: ");
        for (int i = 0; i < length; i++) {
            pos +=sprintf(&buffer[pos],"%02x", id[i]);
        }
        CLogger::getInstance()->log("%s\n",buffer);
    } else {
        CLogger::getInstance()->log("Failed to read device ID");
    }
}

void CBaseThread::startOneShotTimer(uint32_t duration_ms)
{
    // Start the timer with the specified duration (in milliseconds)

    k_timer_start(&my_timer, K_MSEC(duration_ms), K_NO_WAIT);

}

void CBaseThread::stopOneShotTimer()
{
    // Stop the timer
    k_timer_stop(&my_timer);
}

void CBaseThread::timerCallback(struct k_timer *timer_id)
{
    // Timer callback function
    CLogger::getInstance()->log("One-shot timer expired\n");

    CBaseThread* thread = (CBaseThread*)(timer_id->user_data);

    thread->onTimerCallback();

}

void CBaseThread::onTimerCallback()
{
    // Default implementation of the timer callback function
    CLogger::getInstance()->log("Default timer callback function\n");
}

void CBaseThread::setRelease(void)
{
    sprintf(CBaseThread::RELEASE,"%s %2d.%2d","RELEASE",MAJOR_VERSION,MINOR_VERSION);
}

void CBaseThread::read_otp_value(void){

    size_t otp_value;
    int err = 0;
    enum tfm_platform_err_t plt_err;

    err = tfm_ns_interface_init();
    if (err != 0) {
        printk("TF-M non-secure interface initialization failed, error: %d\n", err);
        return;
    }

    plt_err = tfm_platform_mem_read(&otp_value, (intptr_t)&NRF_UICR_S->OTP[0], sizeof(otp_value), &err);
    if (plt_err != TFM_PLATFORM_ERR_SUCCESS || err != 0) {
        printk("Failed to read OTP value. plt_err: %d, err: %d\n", plt_err, err);
    } else {
        printk("OTP[0]: 0x%08X\n", otp_value);
    }

    sprintf(CBaseThread::SERIAL_NUMBER,"%d",otp_value);

}