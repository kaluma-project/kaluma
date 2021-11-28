set(LITTLEFS_DIR ${CMAKE_SOURCE_DIR}/lib/littlefs)
list(APPEND SOURCES
  ${LITTLEFS_DIR}/lfs.c
  ${LITTLEFS_DIR}/lfs_util.c
  ${SRC_DIR}/modules/vfslittlefs/module_vfslittlefs.c)
include_directories(
  ${LITTLEFS_DIR}
  ${SRC_DIR}/modules/vfslittlefs)
