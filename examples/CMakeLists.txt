cmake_minimum_required(VERSION 3.13)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(pico_libsdemu_examples)

pico_sdk_init()

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../ pico-libsdemu)

add_subdirectory(hello_ram_sd)