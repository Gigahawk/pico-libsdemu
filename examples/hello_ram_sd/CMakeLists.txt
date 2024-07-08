cmake_minimum_required(VERSION 3.12)
set(PROJECT hello_ram_sd)

# rest of your project
add_executable(${PROJECT}
    main.c
)

target_link_libraries(${PROJECT} pico-libsdemu)

if(DEFINED CS_PIN)
    message("Setting PICO_DEFAULT_SPI_CSN_PIN to ${CS_PIN}")
    add_compile_definitions(PICO_DEFAULT_SPI_CSN_PIN=${CS_PIN})
endif()

# enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT} 1)
pico_enable_stdio_uart(${PROJECT} 0)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${PROJECT})