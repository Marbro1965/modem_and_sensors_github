// Initializer.h
#ifndef INITIALIZER_H
#define INITIALIZER_H



#ifdef __cplusplus
extern "C" {

    #include <zephyr/kernel.h>
    
#endif



#define     USE_WIFI    1

#define     USE_HELPER_MQTT    1



void initMessageQueue(void);

void initSemaphore(void);

void initialize(void);

//K_THREAD_STACK_DEFINE(thread1_stack, DEFAULT_THREAD_STACK_SIZE);

//struct k_thread thread1_data;

#ifdef __cplusplus
}
#endif

#endif // INITIALIZER_H