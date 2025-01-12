#
# Copyright (c) 2023 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(modem_and_sensor)

# Add the project path to the include directories
include_directories(${CMAKE_SOURCE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/src)

#aggiunta gestione linguaggio C e C++
project(modem_and_sensor LANGUAGES C CXX) 

# Recursively include all .c, .cpp, and .h files in src and its subdirectories
FILE(GLOB_RECURSE app_sources src/*.c src/*.cpp src/*.h src/**/*.c src/**/*.cpp src/**/*.h)

# target_compile_options(app PRIVATE -fpermissive)

# Suppress the error only for the coap.h file
target_compile_options(app PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-fpermissive>)

# NORDIC SDK APP START


target_sources(app PRIVATE ${app_sources})

target_include_directories(app PRIVATE
    src
)

# Conditionally include secure MQTT overlay
if(CONFIG_ENABLE_SECURE_MQTT)
  set(OVERLAY_CONFIG secure_mqtt.conf)
endif()


zephyr_include_directories($ENV{ZEPHYR_BASE}/../nrf/include)

zephyr_include_directories($ENV{ZEPHYR_BASE}/../nrf/include/modem)

zephyr_include_directories($ENV{ZEPHYR_BASE}/../nrf/include/net)

zephyr_include_directories($ENV{ZEPHYR_BASE}/../modules/lib/cjson/)

zephyr_include_directories($ENV{ZEPHYR_BASE}/../modules/lib/date_time/)

zephyr_include_directories($ENV{ZEPHYR_BASE}/../modules/fs/fatfs/include)

zephyr_include_directories($ENV{ZEPHYR_BASE}/../modules/hal_nordic/nrf)

zephyr_include_directories($ENV{ZEPHYR_BASE}/../modules/hal/nordic/nrf)

zephyr_include_directories($ENV{ZEPHYR_BASE}/../drivers)

#zephyr_include_directories($ENV{ZEPHYR_BASE}/include)

zephyr_include_directories($ENV{ZEPHYR_BASE}/subsys/net/lib/sockets)

#zephyr_include_directories($ENV{ZEPHYR_BASE}/../drivers/sensors)




target_link_libraries(app PRIVATE stdc++)

# NORDIC SDK APP END



