#include "CNorHelper.h"

#include "Logger/CLogger.h"

// Implementation of CNorHelper class methods
device * CNorHelper::flash_dev = nullptr;
uint8_t CNorHelper::write_buf[FLASH_TEST_SIZE]={};
uint8_t CNorHelper::read_buf[FLASH_TEST_SIZE]= {};
// Constructor
CNorHelper::CNorHelper() {
    // Initialization code here
}

// Destructor
CNorHelper::~CNorHelper() {
    // Cleanup code here
}


int CNorHelper::writeDataToNor(size_t address, const void *buffer,void *buffer_read, size_t size)
{
    int ret;

    flash_dev = DEVICE_DT_GET(DT_ALIAS(spi_flash0));

    ret = flash_erase(flash_dev, address, 4096);
    if (ret) {
        CLogger::getInstance()->log("Flash erase failed! %d\n", ret);
        return ret;
    }

    ret = flash_write(flash_dev, address, buffer, 4096);

    ret = flash_read(flash_dev, address, buffer_read, 4096);
    if (ret) {
        CLogger::getInstance()->log("Flash read failed! %d\n", ret);
        return ret;
    }



    return 0;

}
// Example method
void CNorHelper::exampleMethod() {
    // Method implementation here
    int ret;

    
    flash_dev = DEVICE_DT_GET(DT_ALIAS(spi_flash0));
    

    if (!device_is_ready(flash_dev)) {
        CLogger::getInstance()->log("Flash device %s is not ready\n", flash_dev->name);
        return;
    }

    // Prepare write buffer
    for (int i = 0; i < FLASH_TEST_SIZE; i++) {
        write_buf[i] = i;
    }

    //Erase flash sector
    ret = flash_erase(flash_dev, FLASH_TEST_OFFSET, FLASH_TEST_SIZE);
    if (ret) {
        CLogger::getInstance()->log("Flash erase failed! %d\n", ret);
        return;
    }

    // Write to flash
    ret = flash_write(flash_dev, FLASH_TEST_OFFSET, write_buf, FLASH_TEST_SIZE);
    if (ret) {
        CLogger::getInstance()->log("Flash write failed! %d\n", ret);
        return;
    }

    // Read from flash
    ret = flash_read(flash_dev, FLASH_TEST_OFFSET, read_buf, FLASH_TEST_SIZE);
    if (ret) {
        CLogger::getInstance()->log("Flash read failed! %d\n", ret);
        return;
    }

    // Verify data
    for (int i = 0; i < FLASH_TEST_SIZE; i++) {
        if (read_buf[i] != write_buf[i]) {
            CLogger::getInstance()->log("Data mismatch at index %d\n", i);
            return;
        }
    }
}