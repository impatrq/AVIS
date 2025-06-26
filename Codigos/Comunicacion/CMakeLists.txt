# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(MotorConNums C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(MotorConNums
    main.c       # El archivo principal que usás ahora
    mpu.c        # El módulo que contiene la lógica del MPU6050
)

# Habilita USB para printf y lectura por consola
pico_enable_stdio_usb(MotorConNums 1)

# Desactiva UART (no se usa)
pico_enable_stdio_uart(MotorConNums 0)

# Vincula todas las librerías necesarias
target_link_libraries(MotorConNums 
    pico_stdlib
    hardware_pwm
    hardware_i2c
)

# Genera los archivos binarios finales
pico_add_extra_outputs(MotorConNums)
