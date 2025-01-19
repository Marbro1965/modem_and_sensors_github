#ifndef CMQTTHELPERTHREAD_H
#define CMQTTHELPERTHREAD_H

#include "BaseThread\CBaseThread.h"

#include <zephyr/kernel.h>

#include <zephyr/drivers/sensor.h>

#include <net/mqtt_helper.h>

#include <stdint.h>


#define JSON_TX_BUFFER_SIZE 1024

#define JSON_RX_BUFFER_SIZE 1024

#define ALIGNMENT 4 

#define MEMORY_POOL_SIZE 1024

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

    int64_t date_time_ms = 0;

    double latitude = 45.52030739893742;  
    double altitude = 120;		      
    double longitude = 9.072246426109826; 

    static CMqttHelperThread *instance;

    char jsonTxBuffer[JSON_TX_BUFFER_SIZE]; // Adjust size as needed based on expected payload size

    char jsonRxBuffer[JSON_RX_BUFFER_SIZE]; // Adjust size as needed based on expected payload size

    static char topicSubscribed[3][64];

    struct mqtt_topic subscribe_topics[3];

    struct mqtt_subscription_list subscription_list;

    struct mqtt_publish_param param;

	bool timerExpired = false;

	void initTopicSubscription();

    static char __aligned(4) memory_pool[MEMORY_POOL_SIZE];

    static size_t memory_pool_index;

    static void *custom_malloc(size_t size);

    static void custom_free(void *ptr);

    //Parsing comando ricevuto

    void parse_json_mqtt_message(char *json_message);


protected:    

    virtual void init_mqtt_helper(void);

    virtual void connect_mqtt(void);

    virtual int prepare_sensor_message(char *jsonTxBuffer);

    virtual int prepare_acknowledge_message(char *jsonBuffer);

    virtual int public_a_message(const char *topic,const char *buffer);

    mqtt_qos qos_publishing = MQTT_QOS_1_AT_LEAST_ONCE;
    
public:

    // constexpr member variable
    static const char *MQTT_BROKER_HOSTNAME;

    static const char *MQTT_TOPIC;

    static const char *MQTT_TOPIC_ACKNOWLEDGE;

    static const char* MQTT_TOPIC_COMANDI_REMOTI;
    
    static const char* MQTT_TOPIC_NEW_RELEASE;

    static const char* MQTT_TOPIC_TEST_OK;



    CMqttHelperThread();

    virtual ~CMqttHelperThread();

    void runHandler(void) override;

    static void on_mqtt_connack(enum mqtt_conn_return_code return_code, bool session_present);

    static void on_mqtt_disconnect(int result);

    static void on_mqtt_publish(struct mqtt_helper_buf topic, struct mqtt_helper_buf payload);

    static void on_mqtt_puback(uint16_t message_id, int result);

    static void on_mqtt_suback(uint16_t message_id, int result);

    static void on_error(enum mqtt_helper_error error);

    void subscribe_to_topic();

	void onTimerCallback() override;

};



#endif // CMQTTHELPER_H