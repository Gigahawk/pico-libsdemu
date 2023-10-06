cmake_minimum_required(VERSION 3.12)
set(PROJECT pico-libsdemu)


# pico_sdk_import.cmake is a single file copied from this SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)


project(${PROJECT})

# initialize the Pico SDK
pico_sdk_init()

add_library(${PROJECT} INTERFACE)

target_sources(${PROJECT} INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/libsdemu.c
)

target_include_directories(${PROJECT} INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/include
)

target_link_libraries(${PROJECT} INTERFACE pico_stdlib hardware_spi)

add_subdirectory("examples/hello_ram_sd")