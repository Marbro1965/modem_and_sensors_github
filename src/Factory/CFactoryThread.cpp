// Define the static member variables
#include "Factory/CFactoryThread.h"

CFactoryThread *CFactoryThread::instance = nullptr;

k_thread CFactoryThread::threadLeds_data;

z_thread_stack_element CFactoryThread::threadLeds_stack[DEFAULT_THREAD_STACK_SIZE];


k_thread CFactoryThread::threadSensor_data;

z_thread_stack_element CFactoryThread::threadSensor_stack[DEFAULT_THREAD_STACK_SIZE];


k_thread CFactoryThread::threadFwUpdater_data;

z_thread_stack_element CFactoryThread::threadFwUpdater_stack[DEFAULT_THREAD_STACK_SIZE];


k_thread CFactoryThread::threadConfigReceiver_data;

z_thread_stack_element CFactoryThread::threadConfigReceiver_stack[DEFAULT_THREAD_STACK_SIZE];


k_thread CFactoryThread::threadLogger_data;

z_thread_stack_element CFactoryThread::threadLogger_stack[DEFAULT_THREAD_STACK_SIZE];


k_thread CFactoryThread::threadModemSetup_data;

z_thread_stack_element CFactoryThread::threadModemSetup_stack[DEFAULT_THREAD_STACK_SIZE];


CFactoryThread::CFactoryThread()
{
    
}


