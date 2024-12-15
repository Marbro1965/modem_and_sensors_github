#ifndef CMQTTHELPERTHREAD_H
#define CMQTTHELPERTHREAD_H

#include "BaseThread\CBaseThread.h"

#include <zephyr/kernel.h>

#include <zephyr/drivers/sensor.h>

#include <net/mqtt_helper.h>

#include <stdint.h>



class CMqttHelperThread: public CBaseThread
{

    enum BROKER_CONNECTION_STATE{
        MQTT_BROKER_STATE_DISCONNECTED,
        MQTT_BROKER_STATE_CONNECTING,
        MQTT_BROKER_STATE_CONNECTED,
        
    };

    enum MQTT_PUBLISH_STATE{
        MQTT_PUBLISH_STATE_IDLE,
        MQTT_PUBLISH_STATE_PUBLISHING,
        MQTT_PUBLISH_STATE_PUBLISHED,
    };

    BROKER_CONNECTION_STATE status = MQTT_BROKER_STATE_DISCONNECTED;

    MQTT_PUBLISH_STATE publish_status = MQTT_PUBLISH_STATE_IDLE;

    //bool sending = true;

    int64_t date_time_ms = 0;

    //char out_vec[200];

    //char *out;

    double latitude = 45.52030739893742;  
    double altitude = 120;		      
    double longitude = 9.072246426109826; 

    static CMqttHelperThread *instance;

    char jsonBuffer[2048]; // Adjust size as needed based on expected payload size

protected:    

    virtual void init_mqtt_helper(void);

    virtual void connect_mqtt(void);

    virtual int publish_message();

    
public:

    // constexpr member variable
    static const char* MQTT_BROKER_HOSTNAME;

    static const char* MQTT_TOPIC;

    CMqttHelperThread();

    virtual ~CMqttHelperThread();

    void runHandler(void) override;

    static void on_mqtt_connack(enum mqtt_conn_return_code return_code, bool session_present);

    static void on_mqtt_disconnect(int result);

    static void on_mqtt_publish(struct mqtt_helper_buf topic, struct mqtt_helper_buf payload);

    static void on_mqtt_puback(uint16_t message_id, int result);

    static void on_mqtt_suback(uint16_t message_id, int result);

    static void on_error(enum mqtt_helper_error error);

};



#endif // CMQTTHELPER_H