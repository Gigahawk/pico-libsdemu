set(PROJECT pico-libsdemu)
add_library(${PROJECT} INTERFACE)

target_sources(${PROJECT} INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/libsdemu.c
)

target_include_directories(${PROJECT} INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/src/include
)

target_link_libraries(${PROJECT} INTERFACE pico_stdlib hardware_spi)
