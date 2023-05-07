list(APPEND SOURCES
    ${SRC_DIR}/modules/pico_cyw43/module_pico_cyw43.c
    ${CMAKE_SOURCE_DIR}/lib/dhcpserver/dhcpserver.c)
include_directories(
    ${SRC_DIR}/modules/pico_cyw43
    ${CMAKE_SOURCE_DIR}/lib/dhcpserver)
