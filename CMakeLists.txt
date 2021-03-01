cmake_minimum_required(VERSION 3.5)

if("${TARGET}" STREQUAL "rpi-pico")
else()
  project(KM_CMAKE NONE)
endif()

# Available targets are in /targets
if(NOT TARGET)
  set(TARGET rpi-pico) # default: rpi-pico
endif()

include(${CMAKE_SOURCE_DIR}/tools/kaluma.cmake)