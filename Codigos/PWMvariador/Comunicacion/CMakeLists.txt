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

# Importa el SDK de la Pico
include(pico_sdk_import.cmake)

# Nombre del proyecto
project(MotorConNums C CXX ASM)

# Estándares de C y C++
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Inicializa el SDK de la Raspberry Pi Pico
pico_sdk_init()

# Agrega el archivo fuente (asegurate que el nombre coincida)
add_executable(MotorConNums
    MotorConNums.c
)

# Habilita stdio por USB (necesario para usar printf y leer por consola USB)
pico_enable_stdio_usb(MotorConNums 1)

# Deshabilita UART (no lo necesitamos en este caso)
pico_enable_stdio_uart(MotorConNums 0)

# Vincula las librerías necesarias
target_link_libraries(MotorConNums pico_stdlib)

# Genera archivos binarios (uf2, bin, elf, etc.)
pico_add_extra_outputs(MotorConNums)
