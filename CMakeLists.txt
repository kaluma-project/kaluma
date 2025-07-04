cmake_minimum_required(VERSION 3.10)

# default target: rp2
if(NOT TARGET)
  set(TARGET "rp2")
endif()

if(NOT "${TARGET}" STREQUAL "rp2")
  project(KM_CMAKE NONE)
endif()

if(NOT VER)
  set(VER "1.3.0-beta.2")
endif()

include(${CMAKE_SOURCE_DIR}/targets/${TARGET}/target.cmake)
configure_file(${CMAKE_SOURCE_DIR}/tools/kaluma_config.h.in ${CMAKE_SOURCE_DIR}/src/gen/kaluma_config.h)
