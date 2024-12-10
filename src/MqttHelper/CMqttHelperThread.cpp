#include "CMqttHelperThread.h"

#include <zephyr/net/mqtt.h>

#include <zephyr/data/json.h>

#include <cJSON.h>

#include <date_time.h>

#include <zephyr/kernel.h>

#include "LoggerThread/CLoggerThread.h"

#define MQTT_BROKER_PORT     1883

#define TIME_DIVISOR	   	1000


CMqttHelperThread* CMqttHelperThread::instance = nullptr;

const char* CMqttHelperThread::MQTT_BROKER_HOSTNAME = "test.mosquitto.org";

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
    
	CLogger::getInstance()->log("MQTT Helper Thread started\n");

    init_mqtt_helper();

    while(true)
    {
        uint32_t events = k_event_wait(&CBaseThread::lte_event_flags, LTE_CONNECTED_FLAG, false, K_FOREVER);

        if (events & LTE_CONNECTED_FLAG) {
            // Handle the event
            CLogger::getInstance()->log("LTE connected event received\n");
			if (STATE_CONNECTED == status) {
				//publish_message();
			} else {
				
			}

		}
        k_sleep(K_SECONDS(10));
    }
}




void CMqttHelperThread::on_mqtt_connack(enum mqtt_conn_return_code return_code, bool session_present)
{
	instance->status = STATE_CONNECTED;
}

void CMqttHelperThread::on_mqtt_disconnect(int result)
{
	instance->status = STATE_DISCONNECTED;
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
	instance->status = STATE_DISCONNECTED;
//	LOG_INF("MQTT ERROR");
	instance->connect_mqtt();

	// smf_set_state(SMF_CTX(&s_obj), &state[MQTT_DISCONNECTED]);
}


int CMqttHelperThread::publish_message()
{
	// struct mqtt_publish_param param;
	//turn_leds_on_with_color(BLUE);
	cJSON *root, *sensor, *value;

	/* create root node and array */
	root = cJSON_CreateObject();
	sensor = cJSON_CreateArray();

	/* add sensors array to root */
	cJSON_AddItemToObject(root, "TF1", sensor);

	if (!date_time_now(&date_time_ms)) {
		date_time_ms /= TIME_DIVISOR;
	} else {
		date_time_ms = k_uptime_get() / TIME_DIVISOR;
	}

	char tempValue[50];
	cJSON_AddItemToArray(sensor, value = cJSON_CreateObject());

	sprintf(tempValue, "%ld", 250001);
	cJSON_AddItemToObject(value, "serial", cJSON_CreateString(tempValue));

	sprintf(tempValue, "%ld", (int32_t)(date_time_ms));
	cJSON_AddItemToObject(value, "time", cJSON_CreateString(tempValue));

	sprintf(tempValue, "%5.0lf", altitude);
	cJSON_AddItemToObject(value, "altitude", cJSON_CreateString(tempValue));

	sprintf(tempValue, "%3.6lf", latitude);
	cJSON_AddItemToObject(value, "latitude", cJSON_CreateString(tempValue));

	sprintf(tempValue, "%3.6lf", longitude);
	cJSON_AddItemToObject(value, "longitude", cJSON_CreateString(tempValue));

	sprintf(tempValue, "%d", iaq.val1);
	cJSON_AddItemToObject(value, "iqa", cJSON_CreateString(tempValue));

	sprintf(tempValue, "%d.%1d", humidity.val1, humidity.val2);
	cJSON_AddItemToObject(value, "humidity", cJSON_CreateString(tempValue));

	sprintf(tempValue, "%d.%1d", temp.val1, temp.val2);
	cJSON_AddItemToObject(value, "ambient_temperature", cJSON_CreateString(tempValue));

	sprintf(tempValue, "%d.%1d", press.val1, press.val2);
	cJSON_AddItemToObject(value, "pressure", cJSON_CreateString(tempValue));

	sprintf(tempValue, "%d.%1d", voc.val1, voc.val2);
	cJSON_AddItemToObject(value, "voc", cJSON_CreateString(tempValue));

	sprintf(tempValue, "%d.%1d", co2.val1, co2.val2);
	cJSON_AddItemToObject(value, "co2", cJSON_CreateString(tempValue));

	out = cJSON_PrintUnformatted(root);

	//LOG_INF("size %d %s \n", strlen(out), out);

	struct mqtt_publish_param param;
	param.message.payload.data = (uint8_t*)out;
	param.message.payload.len = strlen(out);
	param.message.topic.qos = MQTT_QOS_1_AT_LEAST_ONCE;
	param.message_id = k_uptime_get_32();
	param.message.topic.topic.utf8 = (uint8_t *)MQTT_TOPIC;
	param.message.topic.topic.size = strlen(MQTT_TOPIC);
	param.dup_flag = 0;
	param.retain_flag = 0;
	
	int err = 0;
	err = mqtt_helper_publish(&param);
	if (err) {
	//	LOG_WRN("Failed to send payload, err: %d", err);
		// return;
	}
	//turn_leds_off();

	return err;
}