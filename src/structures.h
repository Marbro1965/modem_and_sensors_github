#pragma once

#include <zephyr/drivers/sensor.h>

#define     MSG_SENSOR_MAX_MSGS 10

#define MSGQ_MAX_MSGS 10

#define MSGQ_MAX_MSGS_LOGGER 10

#define MSGQ_ALIGN 4

struct messageSensor{

    sensor_value temp;
    sensor_value press;
    sensor_value humidity;
    sensor_value iaq;
    sensor_value co2;
    sensor_value voc;

};





enum my_msgq_type {
    TURN_LED_GREEN,
    TURN_LED_BLUE,
    TURN_LED_OFF,
    WIFI_CONNECT,
    WIFI_DISCONNECT,
    NOTIFY_NEW_CONFIGURATION,
    NOTIFY_NEW_FIRMWARE,
    TURN_LED_GREEN_BLINKING,
    TURN_LED_BLUE_BLINKING,
};

struct my_msg {

    my_msgq_type data;

};

struct logger_msg{

    char msg[128];

};


extern char __aligned(4) my_msgq_sensor[MSG_SENSOR_MAX_MSGS * sizeof(struct messageSensor)];

extern char __aligned(4) my_msgq_buffer[MSGQ_MAX_MSGS * sizeof(struct my_msg)];

extern char __aligned(4) my_msgq_logger[MSGQ_MAX_MSGS_LOGGER * sizeof(struct logger_msg)];

extern char __aligned(4) my_msgq_leds[10 * sizeof(struct my_msg)];