include(${CMAKE_CURRENT_LIST_DIR}/targets/boards/esp32-devc/target.cmake)

set(_SOURCE_DIR ../../src)

set(_SOURCES
  "${_SOURCE_DIR}/base64.c"
  "${_SOURCE_DIR}/gen/kameleon_magic_strings.c"
  "${_SOURCE_DIR}/gen/kameleon_modules.c"
  "${_SOURCE_DIR}/global.c"
  "${_SOURCE_DIR}/io.c"
  "${_SOURCE_DIR}/jerry_port.c"
  "${_SOURCE_DIR}/jerryxx.c"
  "${_SOURCE_DIR}/main.c"
  "${_SOURCE_DIR}/modules/graphics/font_default.c"
  "${_SOURCE_DIR}/modules/graphics/gc.c"
  "${_SOURCE_DIR}/modules/graphics/gc_16bit_prims.c"
  "${_SOURCE_DIR}/modules/graphics/gc_1bit_prims.c"
  "${_SOURCE_DIR}/modules/graphics/gc_cb_prims.c"
  "${_SOURCE_DIR}/modules/graphics/module_graphics.c"
  "${_SOURCE_DIR}/modules/i2c/module_i2c.c"
  "${_SOURCE_DIR}/modules/pwm/module_pwm.c"
  "${_SOURCE_DIR}/modules/spi/module_spi.c"
  "${_SOURCE_DIR}/modules/storage/module_storage.c"
  "${_SOURCE_DIR}/modules/uart/module_uart.c"
  #"${_SOURCE_DIR}/modules/url/__module_url.c"
  "${_SOURCE_DIR}/repl.c"
  "${_SOURCE_DIR}/runtime.c"
  "${_SOURCE_DIR}/utils.c"
  "${_SOURCE_DIR}/ymodem.c"
)  
set(_INCLUDES
  "../../include"
  "../../include/port"
  "${_SOURCE_DIR}/gen"
  "${_SOURCE_DIR}/modules/graphics"
  "${_SOURCE_DIR}/modules/http"
  "${_SOURCE_DIR}/modules/i2c"
  "${_SOURCE_DIR}/modules/led"
  "${_SOURCE_DIR}/modules/net"
  "${_SOURCE_DIR}/modules/pwm"
  "${_SOURCE_DIR}/modules/spi"
  "${_SOURCE_DIR}/modules/spi"
  "${_SOURCE_DIR}/modules/startup"
  "${_SOURCE_DIR}/modules/storage"
  "${_SOURCE_DIR}/modules/stream"
  "${_SOURCE_DIR}/modules/uart"
  "${_SOURCE_DIR}/modules/url"
  "${_SOURCE_DIR}/modules/wifi")
  
idf_component_register(SRCS
  "esp32_main.c"
  "ieee80211dev.c"
  "pwm_module.c"
  ${TARGET_SOURCE}
  ${_SOURCES}
  INCLUDE_DIRS
  ${_INCLUDES}
  ${TARGET_INCLUDE}
  REQUIRES jerryscript spi_flash console log esp_adc_cal soc nvs_flash)

target_link_libraries(${COMPONENT_LIB} PRIVATE "-lm" "-lc")
target_compile_definitions(${COMPONENT_TARGET} PRIVATE "-D_TARGET_FREERTOS_")

