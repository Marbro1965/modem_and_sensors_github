#include "CModemSetupThread.h"

#include <zephyr/kernel.h> 

//#include <location.h>

#include "structures.h"

#include <zephyr/logging/log.h>

#include <modem/lte_lc.h>

LOG_MODULE_REGISTER(modem_setup_thread, CONFIG_APP_LOG_LEVEL);

CModemSetupThread *CModemSetupThread::instance = nullptr;

CModemSetupThread::CModemSetupThread()
{

    CModemSetupThread::instance = this;

}

CModemSetupThread::~CModemSetupThread()
{

    nrf_modem_lib_shutdown();
}

void CModemSetupThread::runHandler(void)
{
    initModem();

    while(true)
    {
        k_sleep(K_SECONDS(1));
    }
}
void CModemSetupThread::configure_psm() {

    const char *tau = "00000101";         // Example TAU value 5 secondi

    const char *active_time = "00000011"; // Example active time value 3 secondi

    int ret = lte_lc_psm_param_set(tau, active_time);

    if (ret) {
        printk("Failed to set PSM parameters, error: %d\n", ret);
    } else {
        printk("PSM parameters set successfully: TAU=5s, Active Time=5s\n");
    }
}
void CModemSetupThread::initModem(void)
{

    // struct my_msg msg;
    
    // msg.data = TURN_LED_GREEN;

    // int ret = k_msgq_put(&CBaseThread::blinkQueueMessage, &msg, K_NO_WAIT);

   	int err;

    configure_psm();

    err = nrf_modem_lib_init();

    if (err) {
         printk("Failed to initialize modem library: %d", err);   
         return;
    }

    err = lte_lc_connect_async(&CModemSetupThread::lte_handler);

	if (err) {
	 	printk("Failed to connect to LTE network, error: %d\n", err);
	 	return ;
	}

    printk("Connecting async to LTE network\n");

}

// void CModemSetupThread::startLocation(void)
// {
//     int err;

//     // Initialize the Location library
//     struct location_config config;

//     location_config_defaults_set(&config, LOCATION_METHOD_GNSS, 0);

//     config.methods[0].gnss.timeout = 60; // GNSS timeout in seconds

//     err = location_init(&CModemSetupThread::location_event_handler);

//     if (err)
//     {
//         printk("Failed to initialize Location library, error: %d\n", err);
//         return;
//     }

//     // Start obtaining location
//     err = location_request(&config);
//     if (err)
//     {
//         printk("Failed to request location, error: %d\n", err);
//         return;
//     }

//     printk("Location library initialized and location request started\n");
// }

void CModemSetupThread::lte_handler(const struct lte_lc_evt *const evt)
{
	switch (evt->type) {
    	case LTE_LC_EVT_NW_REG_STATUS:
            if (evt->nw_reg_status == LTE_LC_NW_REG_NOT_REGISTERED ||
                evt->nw_reg_status == LTE_LC_NW_REG_SEARCHING ||
                evt->nw_reg_status == LTE_LC_NW_REG_REGISTRATION_DENIED ||
                evt->nw_reg_status == LTE_LC_NW_REG_UNKNOWN) {

                printk("LTE network disconnected"); 

                instance->disconnectFromWiFi(); 

                instance->modem_state = MODEM_STATE_DISCONNECTED;

            } else if (evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME ||
                    evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_ROAMING) {

                printk("LTE network connected");

                instance->connectToWiFi(); 

                printk("Network registration status: %s\n",
                     evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME ? "Connected - home"
                                             : "Connected - roaming");

                instance->modem_state = MODEM_STATE_CONNECTED;            
                
//                instance->startLocation();
    		}
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

        // case LTE_LC_EVT_RSRP_UPDATE:
        //     printk("RSRP update: %d dBm\n", evt->rsrp);
        //     if (evt->rsrp < RSRP_THRESHOLD) {
        //         printf("Warning: Weak signal strength (RSRP=%d dBm)\n", evt->rsrp);
        //     // Add recovery or alert logic here
        //     }
        //     break;            
        // case LTE_LC_EVT_CONNECTED:
        //         printk("Connected to LTE network\n");
        //         break;

        // case LTE_LC_EVT_DISCONNECTED:
        //     printk("Disconnected from LTE network\n");
        //     break;

	    default:
		    break;
	}


}

// void CModemSetupThread::location_event_handler(const struct location_event_data *event_data)
// {
//     if (event_data->id == LOCATION_EVT_LOCATION)
//     {
//         auto lati = event_data->location.latitude;
//         auto longi = event_data->location.longitude;
//         printk("Latitude: %f, Longitude: %f",lati ,longi);
//     }
//     else if (event_data->id == LOCATION_EVT_TIMEOUT)
//     {
//         printk("Location request timed out\n");
//     }
//     else if (event_data->id == LOCATION_EVT_ERROR)
//     {
//         printk("Location request error\n");
//     }
// }


