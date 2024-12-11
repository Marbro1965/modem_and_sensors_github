#include "CModemSetupThread.h"

#include <zephyr/kernel.h> 

//#include <location.h>

#include "structures.h"

#include <zephyr/logging/log.h>

#include <modem/lte_lc.h>

LOG_MODULE_REGISTER(modem_setup_thread, CONFIG_APP_LOG_LEVEL);

CModemSetupThread *CModemSetupThread::instance = nullptr;

//extern struct k_event my_event;

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
    CLogger::getInstance()->log("Modem setup thread started\n");

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
        CLogger::getInstance()->log("Failed to set PSM parameters, error: %d\n", ret);
    } else {
        CLogger::getInstance()->log("PSM parameters set successfully: TAU=5s, Active Time=5s\n");
    }
}
void CModemSetupThread::initModem(void)
{

    struct my_msg msg;
    
    msg.data = TURN_LED_BLUE_BLINKING;

    int ret = k_msgq_put(&CBaseThread::blinkQueueMessage, &msg, K_NO_WAIT);

   	int err;

    //configure_psm();

    err = nrf_modem_lib_init();

    if (err) {
         CLogger::getInstance()->log("Failed to initialize modem library: %d", err);   
         return;
    }

    err = lte_lc_connect_async(&CModemSetupThread::lte_handler);

	if (err) {
	 	CLogger::getInstance()->log("Failed to connect to LTE network, error: %d\n", err);
	 	return ;
	}

    CLogger::getInstance()->log("Connecting async to LTE network\n");

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

    struct my_msg msg;
    
    msg.data = TURN_LED_BLUE_BLINKING;

	switch (evt->type) {
    	case LTE_LC_EVT_NW_REG_STATUS:
            // if ((evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_HOME) &&
            //     (evt->nw_reg_status != LTE_LC_NW_REG_REGISTERED_ROAMING)) {
            //     break;
            // }

            // printk("Network registration status: %s\n",
            //     evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME ? "Connected - home"
            //                             : "Connected - roaming");
            if (evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME ||
                evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_ROAMING) {

                k_event_post(&CBaseThread::lte_event_flags, LTE_CONNECTED_FLAG);

                CLogger::getInstance()->log("LTE connected");
                msg.data = TURN_LED_BLUE;

            } else {

                k_event_post(&CBaseThread::lte_event_flags, LTE_DISCONNECTED_FLAG);

                k_event_clear(&CBaseThread::lte_event_flags, LTE_CONNECTED_FLAG);

                //instance->disconnectFromWiFi();

                CLogger::getInstance()->log("LTE not connected, status: %d", evt->nw_reg_status);
            }

	    	break;
    	case LTE_LC_EVT_PSM_UPDATE:
            CLogger::getInstance()->log("PSM parameter update: TAU: %d s, Active time: %d s\n", evt->psm_cfg.tau,
                 evt->psm_cfg.active_time);
            break;
	    case LTE_LC_EVT_EDRX_UPDATE:
            CLogger::getInstance()->log("eDRX parameter update: eDRX: %.2f s, PTW: %.2f s\n",
                 (double)evt->edrx_cfg.edrx, (double)evt->edrx_cfg.ptw);
            break;
    	case LTE_LC_EVT_RRC_UPDATE:
            CLogger::getInstance()->log("RRC mode: %s\n",
                 evt->rrc_mode == LTE_LC_RRC_MODE_CONNECTED ? "Connected" : "Idle\n");

            if (evt->rrc_mode == LTE_LC_RRC_MODE_CONNECTED)
            {

                msg.data = TURN_LED_BLUE;

            } else{

                msg.data = TURN_LED_OFF;
            }
            break;
    	case LTE_LC_EVT_CELL_UPDATE:
            CLogger::getInstance()->log("LTE cell changed: Cell ID: %d, Tracking area: %d\n", evt->cell.id,
                 evt->cell.tac);
                 
                 
    		break;
	    default:
		    break;
	}

    msg.data = TURN_LED_OFF;

    int ret = k_msgq_put(&CBaseThread::blinkQueueMessage, &msg, K_NO_WAIT);

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


