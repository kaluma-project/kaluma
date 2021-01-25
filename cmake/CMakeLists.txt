cmake_minimum_required(VERSION 3.5)
project(kameleon-project)

# Available targets : kameleon-core linux esp32-devkitc
set(TARGET kameleon-core)

include(${TARGET}.cmake)


