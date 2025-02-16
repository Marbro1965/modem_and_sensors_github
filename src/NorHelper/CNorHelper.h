#pragma once


#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/fs.h>
#include <stdio.h>

#define FLASH_TEST_OFFSET 0x1000
#define FLASH_TEST_SIZE 4096

class CNorHelper
{

    static device *flash_dev;
    static uint8_t write_buf[FLASH_TEST_SIZE];
    static uint8_t read_buf[FLASH_TEST_SIZE];
    

public:

    CNorHelper();

    ~CNorHelper();

    static void exampleMethod();


};