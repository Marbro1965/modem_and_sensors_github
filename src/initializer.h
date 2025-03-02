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

void initMutex(void);

void initSemaphore(void);

void initialize(void);


#ifdef __cplusplus
}
#endif

#endif // INITIALIZER_H