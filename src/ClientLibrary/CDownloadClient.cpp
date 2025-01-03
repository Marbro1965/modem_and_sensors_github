#include "CDownloadClient.h"


size_t CDownloadClient::downloaded=0;

size_t CDownloadClient::file_size = 0;

CDownloadClient* CDownloadClient::instance = nullptr;


CDownloadClient::CDownloadClient()
{
    instance = this;

    config ={};

    buffer_size = 1024;

    buffer = new uint8_t[buffer_size];

    buffer_offset = 0;
}

CDownloadClient::~CDownloadClient()
{
}

void CDownloadClient::init(void)
{
    int err;

    err = download_client_init(&dl, &CDownloadClient::callback);
    if (err) {
        CLogger::getInstance()->log("Failed to initialize the download client, err %d\n", err);
        return;
    }

    err = download_client_get(&dl, URL, &config, URL, 0);
    if (err) {
        CLogger::getInstance()->log("Failed to start download, err %d\n", err);
        return;
    }

    err = download_client_file_size_get(&dl, &file_size);
    if (err) {
        CLogger::getInstance()->log("Failed to get file size, err %d\n", err);
    } else {
        CLogger::getInstance()->log("File size: %d bytes\n", file_size);
    }
}

void CDownloadClient::runHandler(void)
{
    CLogger::getInstance()->log("Download Client Thread started\n");

    uint32_t events = k_event_wait(&CBaseThread::lte_event_flags, LTE_CONNECTED_FLAG, false, K_FOREVER);

    if (events & LTE_CONNECTED_FLAG) {
        
        CLogger::getInstance()->log("LTE connected\n");

        init();
    }
    
    while (true)
    {
        // Sleep for a while before checking again
        k_sleep(K_SECONDS(300)); // Check every hour to synchronize
    }
}


int CDownloadClient::callback(const struct download_client_evt *event)
{
    
    if (event->id == DOWNLOAD_CLIENT_EVT_FRAGMENT) {

        instance->process_fragment((const uint8_t *)event->fragment.buf, event->fragment.len);

        downloaded += event->fragment.len;

        if (file_size) {
            CLogger::getInstance()->log("\rDownloaded %d/%d bytes (%d%%)", downloaded, file_size,
                   (downloaded * 100) / file_size);
        } else {
            CLogger::getInstance()->log("\rDownloaded %d bytes", downloaded);
        }
    } else if (event->id == DOWNLOAD_CLIENT_EVT_DONE) {
        CLogger::getInstance()->log("\nDownload completed in %d ms\n", k_uptime_get_32());
    } else if (event->id == DOWNLOAD_CLIENT_EVT_ERROR) {
        CLogger::getInstance()->log("\nError %d during download\n", event->error);
    }

    return 0;
}

void CDownloadClient::process_fragment(const uint8_t *buf, size_t len)
{
    
    if (buffer_offset + len <= buffer_size) {
        memcpy(buffer + buffer_offset, buf, len);
        buffer_offset += len;
    } else {
        CLogger::getInstance()->log("Buffer overflow, fragment size: %d, buffer offset: %d, buffer size: %d\n", len, buffer_offset, buffer_size);
    }

    // Simulate processing delay
    k_sleep(K_MSEC(100));
    
}