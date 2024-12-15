#include "CLedsThread.h"

#include "structures.h"

#define GPIO_NODE 			DT_NODELABEL(gpio0)

#define RED_LED_NODE        DT_ALIAS(led0)
#define GREEN_LED_NODE      DT_ALIAS(led1)
#define BLUE_LED_NODE       DT_ALIAS(led2)

#define LED_OFF             0
#define LED_ON	            !LED_OFF

const struct device *CLedsThread::gpio_dev = nullptr;

struct gpio_dt_spec CLedsThread::red_led = {};

struct gpio_dt_spec CLedsThread::green_led = {};

struct gpio_dt_spec CLedsThread::blue_led = {};

struct k_timer CLedsThread::my_timer = {};

CLedsThread::CLedsThread()
{

	k_timer_init(&my_timer, timer_expiry_function, timer_stop_function);

	my_timer.user_data = this;
}

CLedsThread::~CLedsThread()
{
  
}

void CLedsThread::init_leds(void)
{
    red_led = GPIO_DT_SPEC_GET(RED_LED_NODE, gpios);
    green_led = GPIO_DT_SPEC_GET(GREEN_LED_NODE, gpios);
    blue_led = GPIO_DT_SPEC_GET(BLUE_LED_NODE, gpios);


	gpio_pin_configure_dt(&red_led, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&green_led, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&blue_led, GPIO_OUTPUT_INACTIVE);
}

void CLedsThread::turn_leds_off(void)
{
	gpio_pin_set_dt(&red_led, LED_OFF);
	gpio_pin_set_dt(&green_led, LED_OFF);
	gpio_pin_set_dt(&blue_led, LED_OFF);
}

void CLedsThread::turn_leds_on_with_color(led_color_t color)
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


int CLedsThread::init(void)
{
    gpio_dev = DEVICE_DT_GET(GPIO_NODE);

	if (!gpio_dev) {

		//printk("Error getting GPIO device binding\r\n");
        return 0;
	}

	init_leds();

    //printk("Leds started\n");

    return 1;
}

void CLedsThread::runHandler(void)
{

	CLogger::getInstance()->log("Esecuzione del thread CLedsThread\n");

	struct my_msg msg;

	int ret = -1;

	int counter = 0;

	bool bBlink = false;

	bool toggle = true;

    init();

	while(true)
    {
		//legge un messaggio dalla coda se c'e'
		ret = k_msgq_get(&CBaseThread::blinkQueueMessage, &msg, K_NO_WAIT);// wait forever for message

		if (ret == 0) 
		{
			//handle message!		
			if (msg.data == TURN_LED_GREEN_BLINKING){

				bBlink = true;
				currentColor = GREEN;
				turn_leds_on_with_color(GREEN);
				// Start the timer with a duration of 1000 ms and a period of 1000 ms
    			k_timer_start(&my_timer, K_MSEC(100), K_MSEC(100));

			} else if (msg.data == TURN_LED_BLUE_BLINKING){

				currentColor = BLUE;
				bBlink = true;
				turn_leds_on_with_color(BLUE);
				// Start the timer with a duration of 1000 ms and a period of 1000 ms
    			k_timer_start(&my_timer, K_MSEC(100), K_MSEC(100));

			}else if (msg.data == TURN_LED_BLUE){

				bBlink = false;
				turn_leds_on_with_color(BLUE);
				k_timer_stop(&my_timer);

			}

			
			if (msg.data == TURN_LED_OFF){
				turn_leds_off();
				k_timer_stop(&my_timer);
			}
		}	

        k_sleep(K_MSEC(10));
    }
}


void CLedsThread::timer_expiry_function(struct k_timer *timer)
{
    // printk("Timer expired\n");
    // Add code to handle timer expiry
	
	
//	CLogger::getInstance()->log("Timer expired\n");

	CLedsThread *me = (CLedsThread*)(timer->user_data);

	if (me->bToggle)
	{
		me->turn_leds_off();
		me->bToggle = false;
	}else
	{
		me->turn_leds_on_with_color(me->currentColor);
		me->bToggle = true;
	}

}

// Timer stop function
void CLedsThread::timer_stop_function(struct k_timer *timer)
{
    // printk("Timer stopped\n");
    // Add code to handle timer stop
}
