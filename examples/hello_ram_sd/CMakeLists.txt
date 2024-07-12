set(PROJECT hello_ram_sd)

add_executable(${PROJECT})

target_sources(${PROJECT} PRIVATE
    main.c
)

target_link_libraries(${PROJECT}
    pico_stdlib
    pico-libsdemu
)

pico_enable_stdio_usb(${PROJECT} 0)
pico_enable_stdio_uart(${PROJECT} 1)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${PROJECT})