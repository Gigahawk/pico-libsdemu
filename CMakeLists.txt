set(PROJECT pico-libsdemu)
add_library(${PROJECT} INTERFACE)

target_sources(${PROJECT} INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/libsdemu.c
)

target_include_directories(${PROJECT} INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/include
)

pico_generate_pio_header(${PROJECT}
    ${CMAKE_CURRENT_LIST_DIR}/src/spisd.pio
)

target_link_libraries(${PROJECT} INTERFACE
    pico_stdlib
    hardware_pio
)
