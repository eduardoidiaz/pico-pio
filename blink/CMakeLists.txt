# add_executable(blink_pio)

# pico_generate_pio_header(blink_pio ${CMAKE_CURRENT_LIST_DIR}/blink.pio)

# target_sources(blink_pio PRIVATE blink.c)

# target_link_libraries(blink_pio PRIVATE
#         pico_stdlib
#         hardware_pio
#         )

# # Pass cmake -DBLINK_PIO_LED_PIN=x, where x is the pin you want to use
# if(BLINK_PIO_LED_PIN)
#         target_compile_definitions(blink_pio PRIVATE
#                 BLINK_PIO_LED_PIN=${BLINK_PIO_LED_PIN}
#         )
# endif()

# pico_add_extra_outputs(blink_pio)

# # add url via pico_set_program_url
# example_auto_set_url(blink_pio)


# cmake version
cmake_minimum_required(VERSION 3.13)

# include the sdk.cmake file
include(../pico_sdk_import.cmake)

# give the project a name (anything you want)
project(PIO_Blink C CXX ASM)

# initialize the sdk
pico_sdk_init()

add_executable(PIO_Blink)

pico_enable_stdio_usb(PIO_Blink 1)
pico_enable_stdio_uart(PIO_Blink 0)

# must match with pio filename and executable name from above
pico_generate_pio_header(PIO_Blink ${CMAKE_CURRENT_LIST_DIR}/blink.pio)

# must match with executable name and source file names
target_sources(PIO_Blink PRIVATE blink.c)

# must match with executable name
target_link_libraries(PIO_Blink PRIVATE pico_stdlib hardware_pio)

# Pass cmake -DBLINK_PIO_LED_PIN=x, where x is the pin you want to use
if(BLINK_PIO_LED_PIN)
        target_compile_definitions(PIO_Blink PRIVATE
                BLINK_PIO_LED_PIN=${BLINK_PIO_LED_PIN}
        )
endif()

# must match with executable name
pico_add_extra_outputs(PIO_Blink)