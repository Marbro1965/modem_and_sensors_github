#include "CDownloadClient.h"

#include <stdio.h>

size_t CDownloadClient::downloaded=0;

size_t CDownloadClient::file_size = 0;

CDownloadClient* CDownloadClient::instance = nullptr;

FATFS CDownloadClient::fat_fs = {};

fs_mount_t CDownloadClient::mp = {};


char CDownloadClient::fileName[32] = {};

struct k_msgq CDownloadClient::download_msgq;
struct download_client_evt CDownloadClient::download_event_buffer[10];

CDownloadClient::CDownloadClient()
{
    instance = this;

    config ={};

    buffer_size = 1024;

    buffer = new uint8_t[buffer_size];

    buffer_offset = 0;

    url = new char[128];


    /* mounting info */
    mp.type = FS_FATFS;

	mp.fs_data = &fat_fs;

    k_msgq_init(&download_msgq, (char *)download_event_buffer, sizeof(struct download_client_evt), 10);
}

CDownloadClient::~CDownloadClient()
{
}

int CDownloadClient::start_file_download(const char *url,const char *file_path)
{
     int err = 0;

   /* Mount the SD card */
    err = mount_sd_card();
    if (err != FR_OK) {
        CLogger::getInstance()->log("SD card mount failed: %d\n", err);
        return err;
    }

    //const char *filename = "/SD:/test.txt";
    /* Delete the file if it already exists */
    sprintf(fileName, "/SD:/%s", file_path);

    while(0!= delete_existing_file(fileName))
    
    if (err) {
        CLogger::getInstance()->log("Failed to prepare file: %d\n", err);
        unmount_sd_card(); // Unmount the SD card in case of an error

        return err;
    }     
    
    /* Initialize the download client */
    err = download_client_init(&dl, &CDownloadClient::callback);
    if (err) {
        CLogger::getInstance()->log("Download client initialization failed: %d\n", err);
        unmount_sd_card(); 
        return err;
    }

    /* Start downloading the file */
    //err = download_client_start(&dl, url, 0);
    err = download_client_get(&dl, url, &config, url, 0);
    if (err) {
        CLogger::getInstance()->log("Download client start failed: %d\n", err);
        unmount_sd_card(); 
        return err;
    }

    CLogger::getInstance()->log("File download started...\n");
    return 0;

}

void CDownloadClient::init(void)
{
    int err;

    err = download_client_init(&dl, &CDownloadClient::callback);
    if (err) {
        CLogger::getInstance()->log("Failed to initialize the download client, err %d\n", err);
        return;
    }


}

void CDownloadClient::download(char *url)
{
    int err;
    err = download_client_get(&dl, url, &config, url, 0);
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
    struct my_msg msg;

    struct download_client_evt event;

    CLogger::getInstance()->log("Download Client Thread started\n");

    // uint32_t events = k_event_wait(&CBaseThread::lte_event_flags, LTE_CONNECTED_FLAG, false, K_FOREVER);

    // if (events & LTE_CONNECTED_FLAG) {
        
    //     CLogger::getInstance()->log("LTE connected\n");

    // }
    
    while (true)
    {

        //k_msgq_get(&download_msgq, &event, K_FOREVER);


        // events = k_event_wait(&CBaseThread::lte_event_flags, LTE_CONNECTED_FLAG, false, K_FOREVER);

        int ret = k_msgq_get(&CBaseThread::msgDownloadClient, &msg, K_NO_WAIT);
        
        if (ret == 0) {
    
            if (msg.data == DOWNLOAD_FIRMWARE) {
    
                start_file_download(URL_FIRMWARE,"firmware.hex");
    
            } else if (msg.data == DOWNLOAD_CONFIG) {
    
                start_file_download(URL_CONFIG,"prova.txt");
            }
        }

        ret = k_msgq_get(&download_msgq, &event, K_NO_WAIT);
        if (ret == 0) {
            process_event(&event);
        }
        

        k_sleep(K_MSEC(10)); // Check every hour to synchronize

    }
}


int CDownloadClient::callback(const struct download_client_evt *event)
{

    k_msgq_put(&download_msgq, event, K_NO_WAIT);

    return 0;
}

void CDownloadClient::process_fragment(const uint8_t *buf, size_t len)
{
    
    // Simulate processing delay
    k_sleep(K_MSEC(100));
    
}

int CDownloadClient::process_event(const struct download_client_evt *event){

    int err;

    fs_file_t file = {};

    switch (event->id) {
        case DOWNLOAD_CLIENT_EVT_FRAGMENT:
            {
            // Write chunk to file (as shown in previous implementation)
            CLogger::getInstance()->log("Received a fragment of size: %d bytes\n", event->fragment.len);
                       
            /* Open the file for appending */
            mount_sd_card();
            err = fs_open(&file,(const char*) &fileName[0], FS_O_CREATE | FS_O_WRITE);
            if (err) {
                CLogger::getInstance()->log("Failed to open file: %d\n", err);
                
                return err;
            }

            /* Move the file pointer to the end for appending */
            err = fs_seek(&file, 0, FS_SEEK_END);
            if (err) {
                CLogger::getInstance()->log("Failed to seek file: %d\n", err);
                fs_close(&file);
               
                return err;
            }

            /* Write the received chunk to the file */
            ssize_t bytes_written = fs_write(&file, event->fragment.buf, event->fragment.len);

            if (bytes_written < 0) {
                CLogger::getInstance()->log("Failed to write to file: %d\n", bytes_written);
                fs_close(&file);
                
                return bytes_written;
            }

            CLogger::getInstance()->log("Successfully wrote %d bytes to the file\n", bytes_written);

            /* Close the file */
            mount_sd_card();

            err = fs_close(&file);
            if (err) {
                CLogger::getInstance()->log("Failed to close file: %d\n", err);
                
                return err;
                }
            
            }
            break;
        case DOWNLOAD_CLIENT_EVT_CLOSED:
        case DOWNLOAD_CLIENT_EVT_DONE:
            {
            struct my_msg msg;

            CLogger::getInstance()->log("Download finished successfully.\n");

            cleanup_after_download(true); // Call cleanup after success

            msg.data = DOWNLOAD_ACK;

-            // download completed
-           k_msgq_put(&CBaseThread::msgAckClient, &msg, K_NO_WAIT);

            }
            break;

        case DOWNLOAD_CLIENT_EVT_ERROR:
            {
            struct my_msg msg;

            CLogger::getInstance()->log("Download failed.\n");

            cleanup_after_download(false); // Call cleanup after failure

            msg.data = DOWNLOAD_NACK;

-            // download completed
-           k_msgq_put(&CBaseThread::msgAckClient, &msg, K_NO_WAIT);

            }
            break;

        default:
            break;
    }

    return 0;
}


int CDownloadClient::mount_sd_card(void)
{
    const char *disk_mount_pt = "/SD:";

    mp.mnt_point = disk_mount_pt;

    int res = fs_mount(&mp);

    // if (res == FR_OK) {
    //     CLogger::getInstance()->log("Disk mounted.\n");
    // } else {
    //     CLogger::getInstance()->log("Error mounting disk.\n");
    // }

    return res;
}

int CDownloadClient::unmount_sd_card(void)
{
    const char *disk_mount_pt = "/SD:";

    mp.mnt_point = disk_mount_pt;

    int err = fs_unmount(&mp);

    // if (err) {
    //     CLogger::getInstance()->log("Failed to unmount SD card: %d\n", err);
    //     return err;
    // }
    // CLogger::getInstance()->log("SD card unmounted successfully.\n");

    return err;
}


int CDownloadClient::delete_existing_file(const char *file_path)
{
    

    int err = fs_unlink(file_path);

    if (err && err != -ENOENT) {
        CLogger::getInstance()->log("Failed to delete existing file: %d\n", err);
        return err;
    }

    if (err == -ENOENT) {
        CLogger::getInstance()->log("File does not exist, no need to delete.\n");
    } else {
        CLogger::getInstance()->log("Existing file deleted successfully.\n");
    }

    return 0;
}

void CDownloadClient::cleanup_after_download(bool success)
{
    if (success) {
        CLogger::getInstance()->log("Download completed successfully.\n");
    } else {
        CLogger::getInstance()->log("Download failed or was interrupted.\n");
    }

    /* Unmount the SD card */
    int err = unmount_sd_card();
    if (err) {
        printk("Failed to unmount SD card: %d\n", err);
    }
}