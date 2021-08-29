cmake_minimum_required(VERSION 3.5)
# Available targets are in /targets
if(NOT TARGET)
  set(TARGET "rpi-pico") # default: rpi-pico
endif()

if(NOT "${TARGET}" STREQUAL "rpi-pico")
  project(KM_CMAKE NONE)
endif()

if(NOT VER)
  set(VER "1.0.0-beta.6")
endif()
configure_file(${CMAKE_SOURCE_DIR}/tools/kaluma_config.h.in ${CMAKE_SOURCE_DIR}/src/gen/kaluma_config.h)
include(${CMAKE_SOURCE_DIR}/targets/${TARGET}/target.cmake)
