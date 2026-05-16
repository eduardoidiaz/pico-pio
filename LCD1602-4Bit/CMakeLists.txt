# cmake version
cmake_minimum_required(VERSION 3.13)

# include the sdk.cmake file
include(../pico_sdk_import.cmake)

# give the project a name (anything you want)
project(Display-4Bit C CXX ASM)

# initialize the sdk
pico_sdk_init()

add_executable(Display-4Bit)

# pico_enable_stdio_usb(Display-4Bit 1)
# pico_enable_stdio_uart(Display-4Bit 0)

# must match with pio filename and executable name from above
pico_generate_pio_header(Display-4Bit ${CMAKE_CURRENT_LIST_DIR}/display-4bit.pio)

# must match with executable name and source file names
target_sources(Display-4Bit PRIVATE display-4bit.c)

# must match with executable name
target_link_libraries(Display-4Bit PRIVATE pico_stdlib pico_bootsel_via_double_reset hardware_pio)

# Pass cmake -DSTART_PIN=x, where x is the pin you want to use
# if(START_PIN)
#         target_compile_definitions(Display-4Bit PRIVATE
#                 START_PIN=${START_PIN}
#         )
# endif()

# must match with executable name
pico_add_extra_outputs(Display-4Bit)