cmake_minimum_required(VERSION 3.5)
project(KM_CMAKE NONE)
set(KM_CMAKE_DIR ${CMAKE_SOURCE_DIR}/tools)
# Available targets : kameleon-core linux

if(NOT TARGET)
  # Use parameter input to set the TARGET
  # cd build
  # cmake .. -DTARGET=linux
  set(TARGET kameleon-core)
endif()

include(${KM_CMAKE_DIR}/kameleon.cmake)