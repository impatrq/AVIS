cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(mpu_sender C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(mpu_sender
    mpu_sender.c
    mpu6050.c
)

target_link_libraries(mpu_sender
    pico_stdlib
    hardware_i2c
    pico_cyw43_arch_lwip_threadsafe_background
)

pico_enable_stdio_usb(mpu_sender 1)
pico_enable_stdio_uart(mpu_sender 0)

pico_add_extra_outputs(mpu_sender)
