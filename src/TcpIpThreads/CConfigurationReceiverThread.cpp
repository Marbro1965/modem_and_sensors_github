#include "CConfigurationReceiverThread.h"


CConfigurationReceiverThread::CConfigurationReceiverThread()
{
    //ctor
}

CConfigurationReceiverThread::~CConfigurationReceiverThread()
{
    //dtor
}

void CConfigurationReceiverThread::runHandler(void)
{
    while(true)
    {
        k_sleep(K_SECONDS(1));
    }

    // int sock;
    // struct sockaddr_in server_addr;
    // const char *server_ip = "your.server.ip.address";
    // const int server_port = 443;

    // sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TLS_1_2);
    // if (sock < 0) {
    //     printk("Failed to create socket: %d\n", errno);
    //     return;
    // }

    // server_addr.sin_family = AF_INET;
    // server_addr.sin_port = htons(server_port);
    // inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    // if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    //     printk("Failed to connect to server: %d\n", errno);
    //     close(sock);
    //     return;
    // }

    // printk("Connected to server\n");

    // // Send and receive data over the socket
    // // ...

    // close(sock);

}

