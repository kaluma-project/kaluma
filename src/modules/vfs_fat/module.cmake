set(OOFATFS_DIR ${CMAKE_SOURCE_DIR}/lib/oofatfs)

list(APPEND SOURCES
  ${OOFATFS_DIR}/src/ff.c
  ${OOFATFS_DIR}/src/ffunicode.c
  ${SRC_DIR}/modules/vfs_fat/vfs_fat.c
  ${SRC_DIR}/modules/vfs_fat/module_vfs_fat.c)

include_directories(
  ${OOFATFS_DIR}/src
  ${SRC_DIR}/modules/vfs_fat)

add_compile_definitions(LFS_NO_ERROR)
add_compile_definitions(LFS_NO_ASSERT)