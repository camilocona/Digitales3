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
set(PICO_BOARD pico CACHE STRING "Board type")

cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(P007_JS_AD)

pico_sdk_init()

add_executable(P007_JS_AD app.c)

pico_add_extra_outputs(P007_JS_AD)

target_include_directories(P007_JS_AD PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(P007_JS_AD hardware_gpio pico_multicore pico_stdlib hardware_uart hardware_irq hardware_i2c)

pico_enable_stdio_usb(P007_JS_AD 1)
pico_enable_stdio_uart(P007_JS_AD 0)

pico_add_extra_outputs(P007_JS_AD)