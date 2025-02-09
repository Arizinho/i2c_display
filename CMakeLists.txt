# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(i2c_display C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(i2c_display i2c_display.c inc/ssd1306.c)

pico_set_program_name(i2c_display "i2c_display")
pico_set_program_version(i2c_display "0.1")

# PIO
pico_generate_pio_header(i2c_display ${CMAKE_CURRENT_LIST_DIR}/ws2812.pio)
target_sources(i2c_display PRIVATE i2c_display.c)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(i2c_display 1)
pico_enable_stdio_usb(i2c_display 1)

# Add the standard library to the build
target_link_libraries(i2c_display
        pico_stdlib hardware_pio hardware_i2c)

# Add the standard include files to the build
target_include_directories(i2c_display PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(i2c_display 
        
        )

pico_add_extra_outputs(i2c_display)

