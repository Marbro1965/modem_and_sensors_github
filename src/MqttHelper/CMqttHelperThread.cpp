#include "CMqttHelperThread.h"

#include <zephyr/net/mqtt.h>

#include <zephyr/data/json.h>

#include <cJSON.h>

#include <date_time.h>

#include <zephyr/kernel.h>

#include "LoggerThread/CLoggerThread.h"

#include "structures.h"

#define MQTT_BROKER_PORT     1883

#define TIME_DIVISOR	   	1000


CMqttHelperThread* CMqttHelperThread::instance = nullptr;

const char* CMqttHelperThread::MQTT_BROKER_HOSTNAME = "93.65.15.223";//"79.55.70.74";     //Brescia"93.65.12.248";

const char* CMqttHelperThread::MQTT_TOPIC  = "bsec/test";

const char* CMqttHelperThread::MQTT_TOPIC_ACKNOWLEDGE  = "bsec/ack";

char CMqttHelperThread::topicSubscribed[3][64] ={};

const char* CMqttHelperThread::MQTT_TOPIC_COMANDI_REMOTI = "/cmds";

const char* CMqttHelperThread::MQTT_TOPIC_NEW_RELEASE = "/verde";

const char* CMqttHelperThread::MQTT_TOPIC_TEST_OK = "/blu";

size_t CMqttHelperThread::memory_pool_index = 0;

char __aligned(4) CMqttHelperThread::memory_pool[MEMORY_POOL_SIZE]={};

CMqttHelperThread::CMqttHelperThread(){

    instance = this;

    initTopicSubscription();

    createTimer();

}

CMqttHelperThread::~CMqttHelperThread(){

}

void CMqttHelperThread::initTopicSubscription(){

    sprintf((char*)topicSubscribed[0], "%s%s", CBaseThread::SERIAL_NUMBER, MQTT_TOPIC_COMANDI_REMOTI);

    sprintf((char*)topicSubscribed[1], "%s%s", CBaseThread::SERIAL_NUMBER, MQTT_TOPIC_NEW_RELEASE);

    sprintf((char*)topicSubscribed[2], "%s%s", CBaseThread::SERIAL_NUMBER, MQTT_TOPIC_TEST_OK);

    subscribe_topics[0].topic.utf8 = (uint8_t*)topicSubscribed[0];
    subscribe_topics[0].topic.size = strlen((const char*)topicSubscribed[0]);
    subscribe_topics[0].qos = MQTT_QOS_1_AT_LEAST_ONCE;

    subscribe_topics[1].topic.utf8 = (uint8_t*)topicSubscribed[1];
    subscribe_topics[1].topic.size = strlen((const char*)topicSubscribed[1]);
    subscribe_topics[1].qos = MQTT_QOS_1_AT_LEAST_ONCE;

    subscribe_topics[2].topic.utf8 = (uint8_t*)topicSubscribed[2];
    subscribe_topics[2].topic.size = strlen((const char*)topicSubscribed[2]);
    subscribe_topics[2].qos = MQTT_QOS_1_AT_LEAST_ONCE;
	
    subscription_list.list = subscribe_topics;
    subscription_list.list_count = ARRAY_SIZE(subscribe_topics);
    subscription_list.message_id = 1234;

}

void CMqttHelperThread::onTimerCallback(){

    timerExpired = true;

    CLogger::getInstance()->log("Timeout connecting to MQTT broker\n");

    status = MQTT_BROKER_STATE_DISCONNECTED;

    connect_mqtt();
}


void CMqttHelperThread::init_mqtt_helper(void)
{
	int err = 0;

    CBaseThread::read_serial_number();

	struct mqtt_helper_cfg cfg = {
		.cb =
			{
			.on_connack = &CMqttHelperThread::on_mqtt_connack,
			.on_disconnect = &CMqttHelperThread::on_mqtt_disconnect,
			.on_publish = &CMqttHelperThread::on_mqtt_publish,
            .on_puback = &CMqttHelperThread::on_mqtt_puback,
			.on_suback = &CMqttHelperThread::on_mqtt_suback,
			.on_error = &CMqttHelperThread::on_error,
			},
	};

	err = mqtt_helper_init(&cfg);
	
	if (err)
	{

		CLogger::getInstance()->log("Errore nell'inizializzazione del client MQTT: %d\n", err);
		return;
	}

	CLogger::getInstance()->log("Client MQTT inizializzato correttamente!\n");


}

void CMqttHelperThread::connect_mqtt(void)
{
	int err = 0;

	struct mqtt_utf8 password = {.utf8 = (const uint8_t *)"", .size = 0};

	struct mqtt_utf8 user = {.utf8 = (const uint8_t *)"", .size = 0};

	/* Avvia la connessione MQTT */
	struct mqtt_helper_conn_params conn_params;

	conn_params.hostname.ptr = (char *)MQTT_BROKER_HOSTNAME;
	conn_params.hostname.size = strlen(MQTT_BROKER_HOSTNAME);
	conn_params.device_id.ptr = (char *)"tfuser123456";
	conn_params.device_id.size = strlen("tfuser123456");
	conn_params.user_name.ptr = (char *)user.utf8;
	conn_params.password.ptr = (char *)password.utf8;
	conn_params.user_name.size = user.size;
	conn_params.password.size = password.size;

	err = mqtt_helper_connect(&conn_params);
	if (err)
	{
		CLogger::getInstance()->log("Failed connecting to MQTT, error code: %d", err);
	}


	startOneShotTimer(20000);

    timerExpired = false;
}

void CMqttHelperThread::runHandler(void){

    int counter = 0;
    
    my_msg msg;

    CLogger::getInstance()->log("MQTT Helper Thread started\n");

    init_mqtt_helper();

    while(true)
    {
        uint32_t events = k_event_wait(&CBaseThread::lte_event_flags, LTE_CONNECTED_FLAG, false, K_FOREVER);

        if (events & LTE_CONNECTED_FLAG) {

            if (MQTT_BROKER_STATE_DISCONNECTED == status)
            {
		        connect_mqtt();

                status = MQTT_BROKER_STATE_CONNECTING;
            }
            else if  (MQTT_BROKER_STATE_CONNECTED == status)
            {

                if (publish_status == MQTT_PUBLISH_STATE_IDLE) {
                    // Handle the event
                    if (qos_publishing== MQTT_QOS_0_AT_MOST_ONCE)
                    {
                        //pubblica senza aspettare risposta
                        publish_status = MQTT_PUBLISH_STATE_IDLE;

                    }else if (qos_publishing== MQTT_QOS_2_EXACTLY_ONCE)
                    {

                        publish_status = MQTT_PUBLISH_STATE_PUBLISHING;

                    }
                    else{

                        publish_status = MQTT_PUBLISH_STATE_IDLE;
                         
                    }
                    if (0==prepare_sensor_message(jsonTxBuffer))
                    {
                        CLogger::getInstance()->log("Publish a message\n");
                        // Prepare MQTT parameters
                        public_a_message(MQTT_TOPIC, jsonTxBuffer);

                    }
                    if (0==prepare_acknowledge_message(jsonTxBuffer))
                    {
                        CLogger::getInstance()->log("Publish a message\n");
                        // Prepare MQTT parameters
                        public_a_message(MQTT_TOPIC_ACKNOWLEDGE, jsonTxBuffer);
                    }

                } 


            }

        }

        k_sleep(K_SECONDS(1));
    }
}

void CMqttHelperThread::subscribe_to_topic(){

    int err = mqtt_helper_subscribe(&subscription_list);
    if (err) {
        
        
    }

}


void CMqttHelperThread::on_mqtt_connack(enum mqtt_conn_return_code return_code, bool session_present)
{

    CLogger::getInstance()->log("Connessione al broker MQTT riuscita!\n");

    instance->stopOneShotTimer();

	instance->status = MQTT_BROKER_STATE_CONNECTED;

    instance->subscribe_to_topic();

}

void CMqttHelperThread::on_mqtt_disconnect(int result)
{
	instance->status = MQTT_BROKER_STATE_DISCONNECTED;
	instance->connect_mqtt();
	
}

void CMqttHelperThread::on_mqtt_publish(struct mqtt_helper_buf topic, struct mqtt_helper_buf payload)
{

    

    CLogger::getInstance()->log("Received a payload\n");

    if (payload.size > 0) {
        // Print the received topic and payload
        CLogger::getInstance()->log("Received topic: %s size: %d", topic.ptr,topic.size);
        CLogger::getInstance()->log("Received payload: %s size %d", payload.ptr,payload.size);

        // Process the payload here
        // For example, you can compare it to known commands:
        if (strncmp(topic.ptr, (const char*)&topicSubscribed[0] , strlen((const char*)&topicSubscribed[0])) == 0){

            snprintf(&(instance->jsonRxBuffer[0]), sizeof(instance->jsonRxBuffer), payload.ptr);

            instance->parse_json_mqtt_message(&(instance->jsonRxBuffer[0]));

            

        }else if  (strncmp(topic.ptr, (const char*)&topicSubscribed[1] , strlen((const char*)&topicSubscribed[1])) == 0){
            

        }else if (strncmp(topic.ptr, (const char*)&topicSubscribed[2] , strlen((const char*)&topicSubscribed[2])) == 0){

            
        }


    } else {

        CLogger::getInstance()->log("Received empty payload");
    }
}

void CMqttHelperThread::on_mqtt_puback(uint16_t message_id, int result)
{
    if (result == 0) {
        instance->publish_status = MQTT_PUBLISH_STATE_IDLE;
    } else {
        
    }
}

void CMqttHelperThread::on_mqtt_suback(uint16_t message_id, int result)
{

    CLogger::getInstance()->log("Subscribe succeed\n");
	// if ((message_id == SUBSCRIBE_TOPIC_ID) && (result == 0)) {
	// 	LOG_INF("Subscribed to topic %s", sub_topic);
	// } else if (result) {
	// 	LOG_ERR("Topic subscription failed, error: %d", result);
	// } else {
	// 	LOG_WRN("Subscribed to unknown topic, id: %d", message_id);
	// }
}

void CMqttHelperThread::on_error(enum mqtt_helper_error error)
{
//	ARG_UNUSED(error);
	instance->status = MQTT_BROKER_STATE_DISCONNECTED;
//	LOG_INF("MQTT ERROR");
	instance->connect_mqtt();

	// smf_set_state(SMF_CTX(&s_obj), &state[MQTT_DISCONNECTED]);
}


int CMqttHelperThread::prepare_sensor_message(char *jsonBuffer)
{
    struct messageSensor msg;
  
    char dataEora[50];

    int jsonIndex = 0;

    int ret = k_msgq_get(&CBaseThread::sensorQueueMessage, &msg, K_NO_WAIT);

    if (0 == ret) {

        // Start building JSON object
        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex, "{");

        if (!date_time_now(&date_time_ms)) {
            date_time_ms /= TIME_DIVISOR;
        } else {
            date_time_ms = k_uptime_get() / TIME_DIVISOR;
        }

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex,
                              "\"TF1\":[{\"serial\":\"%s\",", CBaseThread::SERIAL_NUMBER);

        // jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex,
        //                       "\"time\":\"%ld\",", (int32_t)(date_time_ms));

        struct tm *tm_info = gmtime(&date_time_ms);
        if (tm_info == NULL) {
        
            CLogger::getInstance()->log("Failed to get UTC time");
            return -EINVAL;
        }

        // Print the date and time
        snprintf(dataEora,sizeof(dataEora),"%04d-%02d-%02d %02d:%02d:%02d",
                tm_info->tm_year + 1900,
                tm_info->tm_mon + 1,
                tm_info->tm_mday,
                tm_info->tm_hour,
                tm_info->tm_min,
                tm_info->tm_sec);

        
        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex,
                              "\"readable_time\":\"%s\",",&dataEora[0]);


        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex,
                              "\"altitude\":\"%5.0lf\",", altitude);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex,
                              "\"latitude\":\"%3.6lf\",", latitude);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex,
                              "\"longitude\":\"%3.6lf\",", longitude);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex,
                              "\"iqa\":\"%d\",", msg.iaq.val1);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex,
                              "\"humidity\":\"%d.%1d\",", msg.humidity.val1, msg.humidity.val2);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex,
                              "\"ambient_temperature\":\"%d.%1d\",", msg.temp.val1, msg.temp.val2);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex,
                              "\"pressure\":\"%d.%1d\",", msg.press.val1, msg.press.val2);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex,
                              "\"voc\":\"%d.%1d\",", msg.voc.val1, msg.voc.val2);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex,
                              "\"co2\":\"%d.%1d\"}", msg.co2.val1, msg.co2.val2);

        // Close JSON array and object
        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex, "]}");

        if (jsonIndex >= JSON_TX_BUFFER_SIZE) {
            CLogger::getInstance()->log("JSON buffer overflow");
            return -ENOMEM;
        }
        
    }

    return ret;
}

int CMqttHelperThread::prepare_acknowledge_message(char *jsonBuffer){

    struct my_msg msg;

    int jsonIndex = 0;

    char dataEora[50];

    int ret = k_msgq_get(&CBaseThread::msgAckClient, &msg, K_NO_WAIT);

    if (0==ret)
    {

        if (!date_time_now(&date_time_ms)) {
            date_time_ms /= TIME_DIVISOR;
        } else {
            date_time_ms = k_uptime_get() / TIME_DIVISOR;
        }

        struct tm *tm_info = gmtime(&date_time_ms);
        if (tm_info == NULL) {
        
            CLogger::getInstance()->log("Failed to get UTC time");
            return -EINVAL;
        }

        // Print the date and time
        snprintf(dataEora,sizeof(dataEora),"%04d-%02d-%02d %02d:%02d:%02d",
                tm_info->tm_year + 1900,
                tm_info->tm_mon + 1,
                tm_info->tm_mday,
                tm_info->tm_hour,
                tm_info->tm_min,
                tm_info->tm_sec);


        // Start building JSON object
        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex, "{");

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex,
                            "\"serial\":\"%s\",",CBaseThread::SERIAL_NUMBER);
        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex,
                            "\"readable_time\":\"%s\",",&dataEora[0]);
        if (DOWNLOAD_ACK==msg.data)
        {

            jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex,
                              "\"ack\":\"%s\",","OK");
        }
        else
        {
            jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex,
                              "\"nack\":\"%s\",","OK");

        }
        // Close JSON array and object
        jsonIndex += snprintf(&jsonBuffer[jsonIndex], JSON_TX_BUFFER_SIZE - jsonIndex, "]}");

        if (jsonIndex >= JSON_TX_BUFFER_SIZE) {
            CLogger::getInstance()->log("JSON buffer overflow");
            return -ENOMEM;
        }

    }

    return ret;
}

int CMqttHelperThread::public_a_message(const char *topic,const char *jsonBuffer)
{

    param.message.payload.data = (uint8_t*)jsonBuffer;
    param.message.payload.len = strlen(jsonBuffer);
    param.message.topic.qos = qos_publishing;
    param.message_id = k_uptime_get_32();
    param.message.topic.topic.utf8 = (uint8_t *)topic;
    param.message.topic.topic.size = strlen(topic);
    param.dup_flag = 0;
    param.retain_flag = 0;

    int err = mqtt_helper_publish(&param);
    if (err) {
        CLogger::getInstance()->log("Failed to send payload, err: %d", err);
    }

    return 0;
}
void *CMqttHelperThread::custom_malloc(size_t size) {
     // Calculate the aligned index
    size_t aligned_index = (memory_pool_index + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);

    // Check if there's enough space in the memory pool
    if (aligned_index + size > MEMORY_POOL_SIZE) {
        return NULL; // Not enough memory
    }

    // Allocate memory from the aligned index
    void *ptr = &memory_pool[aligned_index];
    memory_pool_index = aligned_index + size; // Update the memory pool index
    return ptr;
}

void CMqttHelperThread::custom_free(void *ptr) {
    // No-op for static memory pool
}


void CMqttHelperThread::parse_json_mqtt_message(char *json_message)
{

    struct my_msg msg;

    bool messageForLeds = false;

    bool messageForClient = false;

    int ret = 0;

    cJSON_Hooks hooks;
    hooks.malloc_fn = &CMqttHelperThread::custom_malloc;
    hooks.free_fn = &CMqttHelperThread::custom_free;
    cJSON_InitHooks(&hooks);

    // Ensure the input message fits within the buffer size
    if (strlen(json_message) >= JSON_RX_BUFFER_SIZE) {
        
        return;
    }

    // Parse the JSON message in const mode (zero-heap usage)
    cJSON *json = cJSON_ParseWithLengthOpts(json_message, strlen(json_message), NULL, 0);
    if (json == NULL) {
        
        CLogger::getInstance()->log("Failed to parse json message");

        return;
    }

    // Extract and print values directly without dynamic memory usage
    cJSON *cmd = cJSON_GetObjectItemCaseSensitive(json, "cmd");

    if (cJSON_IsString(cmd) && (cmd->valuestring != NULL)) 
    {

        CLogger::getInstance()->log("Found field cmd");

        if (strcmp(cmd->valuestring, "config") == 0) 
        {
            //esegue il download del file di configurazione

            messageForClient = true;

            msg.data = DOWNLOAD_CONFIG;
        }

        if (strcmp(cmd->valuestring, "downloadFirmware") == 0) 
        {
            //comincia le operazioni di download del firmware

            messageForClient = true;

            msg.data = DOWNLOAD_FIRMWARE;
        }

        if (strcmp(cmd->valuestring, "LedBlueOn") == 0) 
        {
            //accende il led Blue
            msg.data = TURN_LED_BLUE;

            messageForLeds = true;
        }

        if (strcmp(cmd->valuestring, "LedGreenOn") == 0) 
        {
            //accende il led verde
            msg.data = TURN_LED_GREEN;

            messageForLeds = true;

        }
        if (strcmp(cmd->valuestring, "LedRedOn") == 0) 
        {
            //accende il led rosso
            msg.data = TURN_LED_RED;

            messageForLeds = true;

        }

        if (strcmp(cmd->valuestring, "LedOff") == 0) 
        {
            //spegne i leds
            msg.data = TURN_LED_OFF;

            messageForLeds = true;

        }

    }

    if (messageForLeds){

        ret = k_msgq_put(&CBaseThread::blinkQueueMessage, &msg, K_NO_WAIT);

        if (ret != 0) {
            CLogger::getInstance()->log("Failed to send message to blink thread");
        }

        msg.data = DOWNLOAD_ACK;

        k_msgq_put(&CBaseThread::msgAckClient, &msg, K_NO_WAIT);
    }

    if (messageForClient){

        ret = k_msgq_put(&CBaseThread::msgDownloadClient, &msg, K_NO_WAIT);

        if (ret != 0) {
            CLogger::getInstance()->log("Failed to send message to download thread");
        }
    }
    

    CMqttHelperThread::memory_pool_index = 0;//equivale a cJSON_Delete(json);

}

void CMqttHelperThread::check_queues_and_set_event() {

    struct messageSensor msg;
    // Check if the sensor queue is not empty
    if (k_msgq_get(&CBaseThread::sensorQueueMessage, &msg, K_NO_WAIT) == 0) {
        k_msgq_put(&CBaseThread::sensorQueueMessage, &msg, K_NO_WAIT); // Put the message back
        k_event_set(&CBaseThread::mqttMessageInQueueFlag, MQTT_MESSAGE_TO_SEND_FLAG);
    }

    // Check if the LEDs queue is not empty
    if (k_msgq_get(&CBaseThread::msgAckClient, &msg, K_NO_WAIT) == 0) {
        k_msgq_put(&CBaseThread::msgAckClient, &msg, K_NO_WAIT); // Put the message back
        k_event_set(&CBaseThread::mqttMessageInQueueFlag, MQTT_MESSAGE_TO_SEND_FLAG);
    }
}