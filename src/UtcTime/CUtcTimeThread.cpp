#include "CUtcTimeThread.h"

#include <zephyr/net/socket.h>
#include <zephyr/net/net_ip.h>
#include <nrf_socket.h>
#include <time.h> 

#define SERVER_ADDRESS "pool.ntp.org" // Example NTP server
#define SERVER_PORT 123


CUtcTimeThread::CUtcTimeThread()
{
}


CUtcTimeThread::~CUtcTimeThread()
{
}

void CUtcTimeThread::runHandler(void) {

    CLogger::getInstance()->log("UTC Time Thread started\n");

    while (true) {

        uint32_t events = k_event_wait(&CBaseThread::lte_event_flags, LTE_CONNECTED_FLAG, false, K_FOREVER);

        if (events & LTE_CONNECTED_FLAG) {

            int err = getUtcTime();

            if (err) {
                CLogger::getInstance()->log("Failed to get UTC time, error code: %d\n", err);
            } else {
                CLogger::getInstance()->log("Successfully retrieved UTC time\n");
            }
        }

        // Sleep for a while before checking again
        k_sleep(K_SECONDS(3600)); // Check every hour to synchronize
    }
}
int CUtcTimeThread::getUtcTime(void) {
    int sock;
    struct nrf_sockaddr_in server_addr;
    char request[48] = {0}; // NTP request packet
    char response[48] = {0}; // NTP response packet

    // Create socket
    sock = nrf_socket(NRF_AF_INET, NRF_SOCK_DGRAM, NRF_IPPROTO_UDP);
    if (sock < 0) {
        CLogger::getInstance()->log("Failed to create socket, error code: %d\n", sock);
        return sock;
    }

    // Configure server address
    struct nrf_in_addr addr;
    if (nrf_inet_pton(NRF_AF_INET, SERVER_ADDRESS, &addr) == 1) {
        // Conversion successful
        server_addr.sin_addr.s_addr = addr.s_addr;
    } else {
    // Conversion failed
    }       
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);


    // Send NTP request
    int err = nrf_sendto(sock, request, sizeof(request), 0, (struct nrf_sockaddr *)&server_addr, sizeof(server_addr));
    if (err < 0) {
        CLogger::getInstance()->log("Failed to send request, error code: %d\n", err);
        nrf_close(sock);
        return err;
    }

    // Receive NTP response
    err = nrf_recv(sock, response, sizeof(response), 0);
    if (err < 0) {
        CLogger::getInstance()->log("Failed to receive response, error code: %d\n", err);
        nrf_close(sock);
        return err;
    }

    // Close socket
    nrf_close(sock);

    // Parse the NTP response to get the UTC time
    uint32_t seconds_since_1900 = (response[40] << 24) | (response[41] << 16) | (response[42] << 8) | response[43];
    uint32_t seconds_since_1970 = seconds_since_1900 - 2208988800U; // Convert to Unix epoch time


    CBaseThread::convertToReadableTime(seconds_since_1970);

    return 0;
}