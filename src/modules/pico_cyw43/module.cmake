list(APPEND SOURCES
    ${SRC_DIR}/modules/pico_cyw43/module_pico_cyw43.c
    ${SRC_DIR}/modules/pico_cyw43/dhcpserver/dhcpserver.c)
include_directories(
    ${SRC_DIR}/modules/pico_cyw43
    ${SRC_DIR}/modules/pico_cyw43/dhcpserver)
