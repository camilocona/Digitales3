# /**
#  * @file CMakeLists.txt
#  * @brief Archivo de configuración del proyecto CMake para Raspberry Pi Pico.
#  * 
#  * Este archivo genera el entorno de compilación para el microcontrolador RP2040,
#  * integrando el SDK de Pico y vinculando bibliotecas requeridas como UART, I2C, GPIO, ADC, etc.
#  * 
#  * @authors
#  * - Camilo Andres Anacona Anacona
#  * - Maria Valentina Quiroga Alzate
#  */

# Requiere como mínimo la versión 3.13 de CMake
cmake_minimum_required(VERSION 3.13)

# Define los estándares de C y C++
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # Exporta comandos de compilación para herramientas como clangd

# ==============================================================================
# Inicializa pico_sdk desde la ubicación instalada
# (puede venir de variables de entorno o caché de CMake)
# ==============================================================================

# == LÍNEAS NECESARIAS para que funcione la extensión de VS Code para Pico ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})     # Ruta HOME para Windows
else()
    set(USERHOME $ENV{HOME})            # Ruta HOME para Linux/macOS
endif()

# Versiones requeridas por la extensión Pico
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)

# Ruta al archivo de configuración de la extensión de VS Code
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)

# Incluye la configuración de la extensión si existe
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ==============================================================================

# Selecciona la placa a usar (en este caso la Raspberry Pi Pico W)
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Importa el SDK de Raspberry Pi Pico (debe ser antes de declarar el proyecto)
include(pico_sdk_import.cmake)

# Define el nombre del proyecto y los lenguajes utilizados
project(app C CXX ASM)

# Inicializa el SDK de Raspberry Pi Pico
pico_sdk_init()

# Agrega el ejecutable principal. El archivo fuente es app.c
add_executable(app app.c )

# Establece el nombre del programa y su versión
pico_set_program_name(app "app")
pico_set_program_version(app "0.1")

# Configura la salida estándar
pico_enable_stdio_uart(app 0)  # Desactiva salida por UART
pico_enable_stdio_usb(app 1)   # Activa salida por USB

# Enlaza bibliotecas estándar al ejecutable
target_link_libraries(app
        pico_stdlib)

# Agrega los directorios de inclusión (headers)
target_include_directories(app PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Enlaza bibliotecas adicionales solicitadas por el usuario
target_link_libraries(app 
        hardware_gpio         # Control de pines GPIO
        pico_multicore        # Soporte para múltiples núcleos
        pico_stdlib           # Biblioteca estándar Pico
        hardware_uart         # Comunicación UART
        hardware_irq          # Interrupciones
        hardware_i2c          # Comunicación I2C
        hardware_adc)         # Conversión analógica a digital

# Genera archivos adicionales de salida (map, bin, uf2, etc.)
pico_add_extra_outputs(app)
