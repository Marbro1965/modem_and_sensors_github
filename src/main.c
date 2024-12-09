/*
 * Copyright (c) 2023 Nordic Semiconductor ASA.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <zephyr/sys/__assert.h>
#include <drivers/bme68x_iaq.h>

#include <zephyr/logging/log.h>

// PROVE MQTT
#include <net/mqtt_helper.h>
#include <zephyr/net/mqtt.h>
#include <zephyr/net/socket.h>
#include <nrf_modem_at.h>
#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>
#include <nrf_modem_at.h>
#include <zephyr/data/json.h>
#include <cJSON.h>

#include <zephyr/net/mqtt.h>
#include <zephyr/random/rand32.h>
#include <zephyr/drivers/gpio.h>

#include "initializer.h"

static void connect_mqtt();

#define GPIO_NODE 			DT_NODELABEL(gpio0)

#define RED_LED_NODE   DT_ALIAS(led0)
#define GREEN_LED_NODE DT_ALIAS(led1)
#define BLUE_LED_NODE  DT_ALIAS(led2)

#define LED_OFF 0
#define LED_ON	!LED_OFF

typedef enum {
	RED,
	GREEN,
	BLUE,
	MAGENTA,
	CYAN,
	YELLOW
} led_color_t;

static const struct device *gpio_dev;

static struct gpio_dt_spec red_led = GPIO_DT_SPEC_GET(RED_LED_NODE, gpios);
static struct gpio_dt_spec green_led = GPIO_DT_SPEC_GET(GREEN_LED_NODE, gpios);
static struct gpio_dt_spec blue_led = GPIO_DT_SPEC_GET(BLUE_LED_NODE, gpios);

void init_leds(void)
{
	gpio_pin_configure_dt(&red_led, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&green_led, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&blue_led, GPIO_OUTPUT_INACTIVE);
}

void turn_leds_off(void)
{
	gpio_pin_set_dt(&red_led, LED_OFF);
	gpio_pin_set_dt(&green_led, LED_OFF);
	gpio_pin_set_dt(&blue_led, LED_OFF);
}

void turn_leds_on_with_color(led_color_t color)
{
	switch (color) {
	case RED:
		gpio_pin_set_dt(&red_led, LED_ON);
		break;
	case GREEN:
		gpio_pin_set_dt(&green_led, LED_ON);
		break;
	case BLUE:
		gpio_pin_set_dt(&blue_led, LED_ON);
		break;
	case MAGENTA:
		gpio_pin_set_dt(&red_led, LED_ON);
		gpio_pin_set_dt(&blue_led, LED_ON);
		break;
	case CYAN:
		gpio_pin_set_dt(&green_led, LED_ON);
		gpio_pin_set_dt(&blue_led, LED_ON);
		break;
	case YELLOW:
		gpio_pin_set_dt(&red_led, LED_ON);
		gpio_pin_set_dt(&green_led, LED_ON);
		break;
	}
}

#define STATE_CONNECTED	   1
#define STATE_DISCONNECTED 0
#define TIME_DIVISOR	   1000

char out_vec[200];
char *out = out_vec;

#define STACK_SIZE 4096
K_THREAD_STACK_DEFINE(thread1_stack, STACK_SIZE);
struct k_thread thread1_data;

K_THREAD_STACK_DEFINE(thread2_stack, STACK_SIZE);
struct k_thread thread2_data;

#define SEND_TIME 30

#define MQTT_BROKER_HOSTNAME "test.mosquitto.org"
#define MQTT_BROKER_PORT     1883

struct mqtt_utf8 password = MQTT_UTF8_LITERAL("");
struct mqtt_utf8 user = MQTT_UTF8_LITERAL("");

#define MQTT_TOPIC   "bsec/test"
#define MQTT_MESSAGE "PROVA STACK"

static uint8_t rx_buffer[128];
static uint8_t tx_buffer[128];

int64_t date_time_ms = 0;

struct sensor_value temp, press, humidity, iaq, co2, voc;

static uint16_t cnt = 0;

uint8_t sending = 0;
uint8_t status = STATE_DISCONNECTED;

double latitude = 45.52030739893742;  // 45.658726;//45.544055;//45.658726;
double altitude = 120;		      // 350.0;//150.0;//350.0;
double longitude = 9.072246426109826; // 10.228208;//10.207329;//10.228208;

static K_SEM_DEFINE(time_update_finished, 0, 1);

static void date_time_evt_handler(const struct date_time_evt *evt)
{
	k_sem_give(&time_update_finished);
}

struct k_mutex sendData;

LOG_MODULE_REGISTER(app, CONFIG_APP_LOG_LEVEL);

K_SEM_DEFINE(lte_connected_sem, 0, 1);

static void lte_handler(const struct lte_lc_evt *const evt)
{
	switch (evt->type) {
	case LTE_LC_EVT_NW_REG_STATUS:
		if ((evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_HOME) &&
		    (evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_ROAMING)) {
			break;
		}

		printk("Network registration status: %s\n",
		       evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME ? "Connected - home"
									   : "Connected - roaming");
		k_sem_give(&lte_connected_sem);
		break;
	case LTE_LC_EVT_PSM_UPDATE:
		printk("PSM parameter update: TAU: %d s, Active time: %d s\n", evt->psm_cfg.tau,
		       evt->psm_cfg.active_time);
		break;
	case LTE_LC_EVT_EDRX_UPDATE:
		printk("eDRX parameter update: eDRX: %.2f s, PTW: %.2f s\n",
		       (double)evt->edrx_cfg.edrx, (double)evt->edrx_cfg.ptw);
		break;
	case LTE_LC_EVT_RRC_UPDATE:
		printk("RRC mode: %s\n",
		       evt->rrc_mode == LTE_LC_RRC_MODE_CONNECTED ? "Connected" : "Idle\n");
		break;
	case LTE_LC_EVT_CELL_UPDATE:
		printk("LTE cell changed: Cell ID: %d, Tracking area: %d\n", evt->cell.id,
		       evt->cell.tac);
		break;
	default:
		break;
	}
}

/* Publish message */
static int publish_message()
{
	// struct mqtt_publish_param param;
	turn_leds_on_with_color(BLUE);
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

	LOG_INF("size %d %s \n", strlen(out), out);

	struct mqtt_publish_param param = {
		.message.payload.data = out,
		.message.payload.len = strlen(out),
		.message.topic.qos = MQTT_QOS_1_AT_LEAST_ONCE,
		.message_id = k_uptime_get_32(),
		.message.topic.topic.utf8 = (uint8_t *)MQTT_TOPIC,
		.message.topic.topic.size = strlen(MQTT_TOPIC),
		.dup_flag = 0,
		.retain_flag = 0,
	};
	int err = 0;
	err = mqtt_helper_publish(&param);
	if (err) {
		LOG_WRN("Failed to send payload, err: %d", err);
		// return;
	}
	turn_leds_off();

	return err;
}

static void on_mqtt_connack(enum mqtt_conn_return_code return_code, bool session_present)
{
	ARG_UNUSED(return_code);

	LOG_INF("MQTT CONENSSO");
	turn_leds_off();
	status = STATE_CONNECTED;

	// smf_set_state(SMF_CTX(&s_obj), &state[MQTT_CONNECTED]);
}

static void on_mqtt_disconnect(int result)
{
	ARG_UNUSED(result);
	status = STATE_DISCONNECTED;
	LOG_INF("MQTT DISCONENSSO");
	connect_mqtt();
	// smf_set_state(SMF_CTX(&s_obj), &state[MQTT_DISCONNECTED]);
}

static void on_mqtt_publish(struct mqtt_helper_buf topic, struct mqtt_helper_buf payload)
{
	LOG_INF("Received payload: %.*s on topic: %.*s", payload.size, payload.ptr, topic.size,
		topic.ptr);
}

static void on_mqtt_suback(uint16_t message_id, int result)
{
	// if ((message_id == SUBSCRIBE_TOPIC_ID) && (result == 0)) {
	// 	LOG_INF("Subscribed to topic %s", sub_topic);
	// } else if (result) {
	// 	LOG_ERR("Topic subscription failed, error: %d", result);
	// } else {
	// 	LOG_WRN("Subscribed to unknown topic, id: %d", message_id);
	// }
}

static void on_error(enum mqtt_helper_error error)
{
	ARG_UNUSED(error);
	status = STATE_DISCONNECTED;
	LOG_INF("MQTT ERROR");
	connect_mqtt();

	// smf_set_state(SMF_CTX(&s_obj), &state[MQTT_DISCONNECTED]);
}

static void connect_mqtt()
{
	int err = 0;
	struct mqtt_helper_cfg cfg = {
		.cb =
			{
				.on_connack = on_mqtt_connack,
				.on_disconnect = on_mqtt_disconnect,
				.on_publish = on_mqtt_publish,
				.on_suback = on_mqtt_suback,
				.on_error = on_error,
			},
	};

	err = mqtt_helper_init(&cfg);
	if (err) {
		printk("Errore nell'inizializzazione del client MQTT: %d\n", err);
		return;
	}
	printk("Client MQTT inizializzato correttamente!\n");

	/* Avvia la connessione MQTT */
	struct mqtt_helper_conn_params conn_params = {
		.hostname.ptr = MQTT_BROKER_HOSTNAME,
		.hostname.size = strlen(MQTT_BROKER_HOSTNAME),
		.device_id.ptr = (uint8_t *)"tfuser123456",
		.device_id.size = strlen("tfuser123456"),
		.user_name.ptr = user.utf8,
		.password.ptr = password.utf8,
		.user_name.size = user.size,
		.password.size = password.size,
	};
	err = mqtt_helper_connect(&conn_params);
	if (err) {
		LOG_ERR("Failed connecting to MQTT, error code: %d", err);
	}
	printk("Connessione al broker MQTT riuscita!\n");
}

void thread1_func(void *arg1, void *arg2, void *arg3)
{
	turn_leds_on_with_color(GREEN);
	printk("Esecuzione del thread 1\n");
	if (IS_ENABLED(CONFIG_DATE_TIME)) {
		/* Registering early for date_time event handler to avoid missing
		 * the first event after LTE is connected.
		 */
		date_time_register_handler(date_time_evt_handler);
	}

	int err;
	err = nrf_modem_lib_init();
	if (err) {
		printk("Failed to initialize modem library, error: %d\n", err);
		return -1;
	}

	err = lte_lc_connect_async(lte_handler);
	if (err) {
		printk("Failed to connect to LTE network, error: %d\n", err);
		return -1;
	}

	k_sem_take(&lte_connected_sem, K_FOREVER);

	if (IS_ENABLED(CONFIG_DATE_TIME)) {
		LOG_INF("Waiting for current time\n");

		/* Wait for an event from the Date Time library. */
		k_sem_take(&time_update_finished, K_MINUTES(10));

		if (!date_time_is_valid()) {
			LOG_INF("Failed to get current time. Continuing anyway.\n");
		}

		if (!date_time_now(&date_time_ms)) {
			LOG_INF("TIME = %lld\n", date_time_ms);
		}
	}

	connect_mqtt();
	/* Gestione loop MQTT */
	while (1) {
		/* Puoi aggiungere altri controlli o logica qui */
		if (sending && status == STATE_CONNECTED) {
			k_mutex_lock(&sendData, K_FOREVER);
			publish_message();
			sending = 0;
			k_mutex_unlock(&sendData);
		}
		k_sleep(K_MSEC(1000));
	}
}

void thread2_func(void *arg1, void *arg2, void *arg3)
{
	const struct device *const dev = DEVICE_DT_GET_ANY(bosch_bme680);
	printk("Esecuzione del thread 2\n");
	if (dev == NULL) {
		LOG_ERR("no device found");
		return 0;
	}

	k_sleep(K_MSEC(5000));
	if (!device_is_ready(dev)) {
		LOG_ERR("device is not ready");
		return 0;
	}
	while (1) {
		k_mutex_lock(&sendData, K_FOREVER);
		turn_leds_on_with_color(RED);
		sensor_sample_fetch(dev);
		sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
		sensor_channel_get(dev, SENSOR_CHAN_PRESS, &press);
		sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &humidity);
		sensor_channel_get(dev, SENSOR_CHAN_IAQ, &iaq);
		sensor_channel_get(dev, SENSOR_CHAN_CO2, &co2);
		sensor_channel_get(dev, SENSOR_CHAN_VOC, &voc);

		LOG_INF("temp: %d.%06d; press: %d.%06d; humidity: %d.%06d; iaq: %d; CO2: %d.%06d; "
			"VOC: %d.%06d",
			temp.val1, temp.val2, press.val1, press.val2, humidity.val1, humidity.val2,
			iaq.val1, co2.val1, co2.val2, voc.val1, voc.val2);
		cnt++;
		if (cnt > (SEND_TIME / 5)) {
			cnt = 0;
			sending = 1;
		}
		k_mutex_unlock(&sendData);
		k_sleep(K_MSEC(1000));
		turn_leds_off();
		k_sleep(K_MSEC(4000));
	}
}

int main(void)
{
	// gpio_dev = DEVICE_DT_GET(GPIO_NODE);

	// if (!gpio_dev) {
	// 	printk("Error getting GPIO device binding\r\n");
	// }
	// init_leds();


	// k_thread_create(&thread1_data, thread1_stack, STACK_SIZE, thread1_func, NULL, NULL, NULL, 1,
	//  		0, K_NO_WAIT);

	// //

	// k_thread_create(&thread2_data, thread2_stack, STACK_SIZE, thread2_func, NULL, NULL, NULL, 2,
	// 		0, K_NO_WAIT);

	initialize();

	// const struct device *const dev = DEVICE_DT_GET_ANY(bosch_bme680);

	// LOG_INF("App started");

	return 0;
}
