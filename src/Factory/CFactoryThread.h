#ifndef CFACTORY_THREAD_H
#define CFACTORY_THREAD_H

#include "Leds/CLedsThread.h"

#include "SensorThread/CSensorThread.h"

#include "TcpIpThreads/CFirmwareUpdaterThread.h"

#include "TcpIpThreads/CConfigurationReceiverThread.h"

#include "LoggerThread/CLoggerThread.h"

#include "NetworkService/CModemSetupThread.h"

class CFactoryThread
{

private:

    static CFactoryThread *instance;

    static k_thread threadLeds_data;

    static z_thread_stack_element threadLeds_stack[DEFAULT_THREAD_STACK_SIZE];

    static k_thread threadSensor_data;

    static z_thread_stack_element threadSensor_stack[DEFAULT_THREAD_STACK_SIZE];

    static k_thread threadFwUpdater_data;

    static z_thread_stack_element threadFwUpdater_stack[DEFAULT_THREAD_STACK_SIZE];

    static k_thread threadConfigReceiver_data;

    static z_thread_stack_element threadConfigReceiver_stack[DEFAULT_THREAD_STACK_SIZE];

    static k_thread threadLogger_data;

    static z_thread_stack_element threadLogger_stack[DEFAULT_THREAD_STACK_SIZE];

    static k_thread threadModemSetup_data;

    static z_thread_stack_element threadModemSetup_stack[DEFAULT_THREAD_STACK_SIZE];


    CFactoryThread();

public:

    static CFactoryThread *getInstance(){

        if(instance == nullptr)
        {
            instance = new CFactoryThread();
        }

        return instance;
    }

    void createThreads(void){

        //Creazione del Thread per i leds

        // CLedsThread *pLedsThread = new CLedsThread();

        // k_tid_t id1 = k_thread_create(&CFactoryThread::threadLeds_data,CFactoryThread::threadLeds_stack, 
        //                                                 DEFAULT_THREAD_STACK_SIZE, pLedsThread->handlerRun, pLedsThread,
        //                                                  NULL, NULL, 10, 0, K_NO_WAIT);


        // CSensorThread *pSensorThread = new CSensorThread();

        // k_tid_t id2 = k_thread_create(&CFactoryThread::threadSensor_data,CFactoryThread::threadSensor_stack,
        //                                                 DEFAULT_THREAD_STACK_SIZE, pSensorThread->handlerRun, pSensorThread,
        //                                                  NULL, NULL, 10, 0, K_NO_WAIT);


        // CLoggerThread *pLoggerThread = new CLoggerThread();

        // k_tid_t id3 = k_thread_create(&CFactoryThread::threadLogger_data,CFactoryThread::threadLogger_stack, 
        //                                                 DEFAULT_THREAD_STACK_SIZE, pLoggerThread->handlerRun, pLoggerThread, 
        //                                                 NULL, NULL, 10, 0, K_NO_WAIT);


        CModemSetupThread *pModemSetupThread = new CModemSetupThread();

        k_tid_t id4 = k_thread_create(&CFactoryThread::threadModemSetup_data,CFactoryThread::threadModemSetup_stack, 
                                                        DEFAULT_THREAD_STACK_SIZE, pModemSetupThread->handlerRun, pModemSetupThread, 
                                                        NULL, NULL, 10, 0, K_NO_WAIT);


    }

};

#endif