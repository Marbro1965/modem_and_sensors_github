#pragma once

#include "BaseThread/CBaseThread.h"

#include <zephyr/net/coap.h>

#include <net/download_client.h>

#define URL "http://panel.bbmold.com/firmware/release.txt"


class CDownloadClient : public CBaseThread
{

    static CDownloadClient *instance;

    download_client dl;

    download_client_cfg config;

    static size_t file_size;

    static size_t downloaded;

    void init();

    uint8_t *buffer;

    size_t buffer_size;

    size_t buffer_offset;

public:
    CDownloadClient();

    ~CDownloadClient();

    void runHandler(void) override;

    static int callback(const struct download_client_evt *event);

    void process_fragment(const uint8_t *buf, size_t len);

    // Wrapper function for coap_bytes_to_block_size
    coap_block_size coap_bytes_to_block_size_wrapper(uint16_t bytes);
    
};