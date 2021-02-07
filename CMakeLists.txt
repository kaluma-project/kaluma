cmake_minimum_required(VERSION 3.5)
if("${TARGET}" STREQUAL "rpi-pico")
else()
  project(KM_CMAKE NONE)
endif()
set(KM_CMAKE_DIR ${CMAKE_SOURCE_DIR}/tools)
# Available targets : kameleon-core linux

if(NOT TARGET)
  # Use parameter input to set the TARGET
  # cd build
  # cmake .. -DTARGET=linux
  set(TARGET kameleon-core)
endif()

include(${KM_CMAKE_DIR}/kalamu.cmake)