cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(pico_usb_echo C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(pico_usb_echo
    EchoPico.c
)

target_link_libraries(pico_usb_echo
    pico_stdlib
    pico_stdio_usb
)

pico_add_extra_outputs(pico_usb_echo)