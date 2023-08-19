list(APPEND SOURCES
  ${SRC_DIR}/modules/graphics/gc_cb_prims.c
  ${SRC_DIR}/modules/graphics/gc_1bit_prims.c
  ${SRC_DIR}/modules/graphics/gc_3bit_prims.c
  ${SRC_DIR}/modules/graphics/gc_16bit_prims.c
  ${SRC_DIR}/modules/graphics/gc.c
  ${SRC_DIR}/modules/graphics/font_default.c
  ${SRC_DIR}/modules/graphics/module_graphics.c)
include_directories(${SRC_DIR}/modules/graphics)
