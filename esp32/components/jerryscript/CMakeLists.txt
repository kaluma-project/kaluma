set(_JERRY_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../deps/jerryscript")

idf_component_register(
  INCLUDE_DIRS
  "${_JERRY_DIR}/jerry-core/include"
  "${_JERRY_DIR}/jerry-ext/include"
  "${_JERRY_DIR}/jerry-libm/include"
  "${_JERRY_DIR}/jerry-port/default/include"
  "${_JERRY_DIR}/targets/curie_bsp/include"
  "${_JERRY_DIR}/targets/curie_bsp/jerry_app/include")

target_link_libraries(${COMPONENT_LIB} INTERFACE
  "${_JERRY_DIR}/build/lib/libjerry-core.a" "${_JERRY_DIR}/build/lib/libjerry-ext.a" "${_JERRY_DIR}/build/lib/libjerry-port-default.a")
