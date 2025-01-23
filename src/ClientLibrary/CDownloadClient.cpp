#include "CDownloadClient.h"

#include <stdio.h>

#include <string.h>

CDownloadClient* CDownloadClient::instance = nullptr;

FATFS CDownloadClient::fat_fs = {};

fs_mount_t CDownloadClient::mp = {};

char CDownloadClient::fileName[32] = {};

struct k_msgq CDownloadClient::download_msgq;

struct download_client_evt CDownloadClient::download_event_buffer[10];

uint8_t CDownloadClient::cleaned_data[CONFIG_DOWNLOAD_CLIENT_BUF_SIZE] = {};

size_t CDownloadClient::cleaned_len = 0;


CDownloadClient::CDownloadClient()
{
    instance = this;

    config ={};

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
    sprintf(fileName, "/SD:/%s", file_path);

    // Delete the file if it already exists
    while(0!= delete_existing_file(fileName))
    
    // if (err) {
    //     CLogger::getInstance()->log("Failed to prepare file: %d\n", err);
    //     unmount_sd_card(); // Unmount the SD card in case of an error

    //     return err;
    // }     
    
    /* Initialize the download client */
    err = download_client_init(&dl, &CDownloadClient::callback);
    if (err) {
        CLogger::getInstance()->log("Download client initialization failed: %d\n", err);
        unmount_sd_card(); 
        return err;
    }

    /* Start downloading the file */
    err = download_client_get(&dl, url, &config, url, 0);
    if (err) {
        CLogger::getInstance()->log("Download client start failed: %d\n", err);
        unmount_sd_card(); 
        return err;
    }

    CLogger::getInstance()->log("File download started...\n");
    return 0;

}



void CDownloadClient::runHandler(void)
{
    struct my_msg msg;

    struct download_client_evt event;

    CLogger::getInstance()->log("Download Client Thread started\n");

    while (true)
    {

        int ret = k_msgq_get(&CBaseThread::msgDownloadClient, &msg, K_NO_WAIT);
        
        if (ret == 0) {
    
            if (msg.data == DOWNLOAD_FIRMWARE) {
    
                start_file_download(URL_FIRMWARE,"firmware.hex");
    
            } else if (msg.data == DOWNLOAD_CONFIG) {
    
                start_file_download(URL_CONFIG,"prova.txt");
            }
        }

        // ret = k_msgq_get(&download_msgq, &event, K_NO_WAIT);
        // if (ret == 0) {
        //     process_event(&event);
        // }
        

        k_sleep(K_MSEC(10)); // Check every hour to synchronize

    }
}


int CDownloadClient::callback(const struct download_client_evt *event)
{

    //k_msgq_put(&download_msgq, event, K_NO_WAIT);

    process_event(event);

    return 0;
}


int CDownloadClient::process_event(const struct download_client_evt *event){

    int err;

    fs_file_t file = {};

    switch (event->id) {
        case DOWNLOAD_CLIENT_EVT_FRAGMENT:
            {
            // Write chunk to file (as shown in previous implementation)
            CLogger::getInstance()->log("Received a fragment of size: %d bytes\n", event->fragment.len);
                       
            cleaned_len = 0;

            remove_headers(event->fragment.buf, event->fragment.len, cleaned_data, &cleaned_len);           
            /* Open the file for appending */
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
            ssize_t bytes_written = fs_write(&file, cleaned_data, cleaned_len);

            if (bytes_written < 0) {
                CLogger::getInstance()->log("Failed to write to file: %d\n", bytes_written);
                fs_close(&file);
                
                return bytes_written;
            }

            CLogger::getInstance()->log("Successfully wrote %d bytes to the file\n", bytes_written);

            /* Close the file */
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

void *CDownloadClient::custom_memmem(const void *haystack, size_t haystacklen, const void *needle, size_t needlelen) {

    if (needlelen == 0) {
        return (void *)haystack;
    }

    const char *h = (const char *)haystack;
    const char *n = (const char *)needle;

    for (size_t i = 0; i <= haystacklen - needlelen; i++) {
        if (h[i] == n[0] && memcmp(&h[i], n, needlelen) == 0) {
            return (void *)&h[i];
        }
    }

    return NULL;
}

void CDownloadClient::remove_headers(const void *input, size_t input_len, uint8_t *output, size_t *output_len) {

    const char *header_end = (const char *)custom_memmem(input, input_len, "\r\n\r\n", 4);
    if (header_end) {
        size_t header_size = header_end - (char *)input + 4;
        *output_len = input_len - header_size;
        memcpy(output, input + header_size, *output_len);
    } else {
        // If we can't find the header end, copy everything (this shouldn't happen in practice)
        *output_len = input_len;
        memcpy(output, input, input_len);
    }
}
