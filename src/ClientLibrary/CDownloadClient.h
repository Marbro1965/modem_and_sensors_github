#pragma once

#include "BaseThread/CBaseThread.h"

#include <zephyr/net/coap.h>

#include <net/download_client.h>

#define URL_CONFIG "http://panel.bbmold.com/config/rel00.config"

#define URL_FIRMWARE "http://panel.bbmold.com/firmware/merged.hex"

#include <zephyr/storage/disk_access.h>

#include <zephyr/fs/fs.h>

#include <ff.h>

#include <zephyr/kernel.h> 

#define DISK_DRIVE_NAME "SD"

#define DISK_MOUNT_PT "/"DISK_DRIVE_NAME":"

#define CONFIG_DOWNLOAD_CLIENT_BUF_SIZE 2048

class CDownloadClient : public CBaseThread
{

    static CDownloadClient *instance;

    download_client dl;

    download_client_cfg config;

    static uint8_t cleaned_data[CONFIG_DOWNLOAD_CLIENT_BUF_SIZE];

    static size_t cleaned_len;

    char *url;

    static char fileName[32];

    static FATFS fat_fs;

    static fs_mount_t mp;

    int start_file_download(const char *url,const char* file_path);

    static int mount_sd_card(void);

    static int unmount_sd_card(void);

    static int delete_existing_file(const char *file_path);

    static void cleanup_after_download(bool success);

    static struct k_msgq download_msgq;

    static struct download_client_evt download_event_buffer[10];

    static int process_event(const struct download_client_evt *event);

    static void *custom_memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen);

    static void remove_headers(const void *input, size_t input_len, uint8_t *output, size_t *output_len);

public:

    CDownloadClient();

    ~CDownloadClient();

    void runHandler(void) override;

    static int callback(const struct download_client_evt *event);

    
};