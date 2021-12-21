set(LITTLEFS_DIR ${CMAKE_SOURCE_DIR}/lib/littlefs)

list(APPEND SOURCES
  ${LITTLEFS_DIR}/lfs.c
  ${LITTLEFS_DIR}/lfs_util.c
  ${SRC_DIR}/modules/vfs_lfs/vfs_lfs.c
  ${SRC_DIR}/modules/vfs_lfs/module_vfs_lfs.c)

include_directories(
  ${LITTLEFS_DIR}
  ${SRC_DIR}/modules/vfs_lfs)

add_compile_definitions(LFS_NO_ERROR)
add_compile_definitions(LFS_NO_ASSERT)
