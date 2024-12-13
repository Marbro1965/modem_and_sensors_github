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

const char* CMqttHelperThread::MQTT_BROKER_HOSTNAME = "93.65.12.248";

const char* CMqttHelperThread::MQTT_TOPIC  = "bsec/test";

CMqttHelperThread::CMqttHelperThread(){

    instance = this;
	
}

CMqttHelperThread::~CMqttHelperThread(){

}


void CMqttHelperThread::init_mqtt_helper(void)
{
	int err = 0;

	struct mqtt_helper_cfg cfg = {
		.cb =
			{
			.on_connack = &CMqttHelperThread::on_mqtt_connack,
			.on_disconnect = &CMqttHelperThread::on_mqtt_disconnect,
			.on_publish = &CMqttHelperThread::on_mqtt_publish,
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
	
	CLogger::getInstance()->log("Connessione al broker MQTT riuscita!\n");
}

void CMqttHelperThread::runHandler(void){
    
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
            }

            if (MQTT_BROKER_STATE_CONNECTED == status) {
                // Handle the event
                CLogger::getInstance()->log("Publish a message\n");
                publish_message();
            } else 
            {
                connect_mqtt();
            }
        }

        k_sleep(K_SECONDS(1));
    }
}




void CMqttHelperThread::on_mqtt_connack(enum mqtt_conn_return_code return_code, bool session_present)
{
	instance->status = MQTT_BROKER_STATE_CONNECTED;
}

void CMqttHelperThread::on_mqtt_disconnect(int result)
{
	instance->status = MQTT_BROKER_STATE_DISCONNECTED;
	instance->connect_mqtt();
	
}

void CMqttHelperThread::on_mqtt_publish(struct mqtt_helper_buf topic, struct mqtt_helper_buf payload)
{
//	LOG_INF("Received payload: %.*s on topic: %.*s", payload.size, payload.ptr, topic.size,
//		topic.ptr);
}

void CMqttHelperThread::on_mqtt_suback(uint16_t message_id, int result)
{
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


int CMqttHelperThread::publish_message()
{
    struct messageSensor msg;
  

    char dataEora[50];
    int jsonIndex = 0;

    int ret = k_msgq_get(&CBaseThread::sensorQueueMessage, &msg, K_NO_WAIT);

    if (0 == ret) {

        // Start building JSON object
        jsonIndex += snprintf(&jsonBuffer[jsonIndex], sizeof(jsonBuffer) - jsonIndex, "{");

        if (!date_time_now(&date_time_ms)) {
            date_time_ms /= TIME_DIVISOR;
        } else {
            date_time_ms = k_uptime_get() / TIME_DIVISOR;
        }

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], sizeof(jsonBuffer) - jsonIndex,
                              "\"TF1\":[{\"serial\":\"%ld\",", 250001);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], sizeof(jsonBuffer) - jsonIndex,
                              "\"time\":\"%ld\",", (int32_t)(date_time_ms));

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

        
        jsonIndex += snprintf(&jsonBuffer[jsonIndex], sizeof(jsonBuffer) - jsonIndex,
                              "\"readable_time\":\"%s\",",&dataEora[0]);


        jsonIndex += snprintf(&jsonBuffer[jsonIndex], sizeof(jsonBuffer) - jsonIndex,
                              "\"altitude\":\"%5.0lf\",", altitude);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], sizeof(jsonBuffer) - jsonIndex,
                              "\"latitude\":\"%3.6lf\",", latitude);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], sizeof(jsonBuffer) - jsonIndex,
                              "\"longitude\":\"%3.6lf\",", longitude);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], sizeof(jsonBuffer) - jsonIndex,
                              "\"iqa\":\"%d\",", msg.iaq.val1);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], sizeof(jsonBuffer) - jsonIndex,
                              "\"humidity\":\"%d.%1d\",", msg.humidity.val1, msg.humidity.val2);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], sizeof(jsonBuffer) - jsonIndex,
                              "\"ambient_temperature\":\"%d.%1d\",", msg.temp.val1, msg.temp.val2);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], sizeof(jsonBuffer) - jsonIndex,
                              "\"pressure\":\"%d.%1d\",", msg.press.val1, msg.press.val2);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], sizeof(jsonBuffer) - jsonIndex,
                              "\"voc\":\"%d.%1d\",", msg.voc.val1, msg.voc.val2);

        jsonIndex += snprintf(&jsonBuffer[jsonIndex], sizeof(jsonBuffer) - jsonIndex,
                              "\"co2\":\"%d.%1d\"}", msg.co2.val1, msg.co2.val2);

        // Close JSON array and object
        jsonIndex += snprintf(&jsonBuffer[jsonIndex], sizeof(jsonBuffer) - jsonIndex, "]}");

        if (jsonIndex >= sizeof(jsonBuffer)) {
            CLogger::getInstance()->log("JSON buffer overflow");
            return -ENOMEM;
        }

        // Prepare MQTT parameters
        struct mqtt_publish_param param;
        param.message.payload.data = (uint8_t*)jsonBuffer;
        param.message.payload.len = strlen(jsonBuffer);
        param.message.topic.qos = MQTT_QOS_1_AT_LEAST_ONCE;
        param.message_id = k_uptime_get_32();
        param.message.topic.topic.utf8 = (uint8_t *)MQTT_TOPIC;
        param.message.topic.topic.size = strlen(MQTT_TOPIC);
        param.dup_flag = 0;
        param.retain_flag = 0;

        int err = mqtt_helper_publish(&param);
        if (err) {
            CLogger::getInstance()->log("Failed to send payload, err: %d", err);
        }
    }

    return ret;
}
