#include "CWdtTestKernelPanic.h"



#define GPIO_NODE 			DT_NODELABEL(gpio0)

#define BUTTON_NODE    		DT_ALIAS(sw0)


CWdtTestKernelPanic* CWdtTestKernelPanic::instance = nullptr;

const struct device *CWdtTestKernelPanic::gpio_dev = nullptr;

struct gpio_dt_spec CWdtTestKernelPanic::button = {};

struct gpio_callback CWdtTestKernelPanic::gpio_cb={};
    


CWdtTestKernelPanic::CWdtTestKernelPanic()
{
    instance = this;
}


CWdtTestKernelPanic::~CWdtTestKernelPanic()
{
    
}

void CWdtTestKernelPanic::button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins) {
    
    instance->crash_function((uint32_t *)0x0);

}

bool CWdtTestKernelPanic::init(void)
{

    gpio_dev = DEVICE_DT_GET(GPIO_NODE);

	button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

	int ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0) {
		CLogger::getInstance()->log("Error %d: failed to configure %s pin %d\n",
				ret, button.port->name, button.pin);
		
		return false;
	}

	ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		CLogger::getInstance()->log("Error %d: failed to configure interrupt on %s pin %d\n",
			   ret, button.port->name, button.pin);

		return false;
	}

	gpio_init_callback(&gpio_cb, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &gpio_cb);

	return true;

}

void CWdtTestKernelPanic::crash_function(uint32_t *addr)
{

	#if !defined(CONFIG_CPU_CORTEX_M)
	/* For null pointer reference */
	*addr = 0;
	#else
		ARG_UNUSED(addr);
		/* Dereferencing null-pointer in TrustZone-enabled
	 	* builds may crash the system, so use, instead an
	 	* undefined instruction to trigger a CPU fault.
	 	*/
	__asm__ volatile("udf #0" : : : );
#endif
}

void CWdtTestKernelPanic::runHandler(void)
{
    init();
    
    while(true)
    {
        k_sleep(K_FOREVER);
    }
}
 