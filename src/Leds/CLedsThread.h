#pragma once

#include "BaseThread/CBaseThread.h"

#include <zephyr/kernel.h>

#include <zephyr/device.h>

#include <zephyr/drivers/gpio.h>

typedef enum {
	RED,
	GREEN,
	BLUE,
	MAGENTA,
	CYAN,
	YELLOW
} led_color_t;

class CLedsThread : public CBaseThread
{

    static const struct device *gpio_dev;

    static struct gpio_dt_spec red_led;
    static struct gpio_dt_spec green_led;
    static struct gpio_dt_spec blue_led;

    static struct k_timer my_timer;

    void init_leds(void);

    void turn_leds_off(void);

    void turn_leds_on_with_color(led_color_t color);

    int init(void);

    
    
public:
        
    CLedsThread();
    
    ~CLedsThread();
    
    void runHandler(void) override;

    // Timer callback functions
    static void timer_expiry_function(struct k_timer *timer);
    static void timer_stop_function(struct k_timer *timer);

    bool bToggle = false;
};