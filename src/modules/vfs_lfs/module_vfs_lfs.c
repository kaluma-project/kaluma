/* Copyright (c) 2017 Kaluma
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "module_vfs_lfs.h"

#include <stdlib.h>

#include "err.h"
#include "io.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "lfs.h"
#include "tty.h"  // for tty_printf()
#include "vfs_lfs.h"
#include "vfs_lfs_magic_strings.h"

static void vfs_handle_freecb(void *handle) {
  vfs_lfs_handle_t *vfs_handle = (vfs_lfs_handle_t *)handle;
  jerry_release_value(vfs_handle->blockdev_js);
  free(handle);
}

static const jerry_object_native_info_t vfs_handle_info = {
    .free_cb = vfs_handle_freecb};

static int bd_ioctl(jerry_value_t blockdev_js, int op, int arg) {
  km_tty_printf("bd_ioctl(%d, %d)\r\n", op, arg);
  jerry_value_t ioctl_js =
      jerryxx_get_property(blockdev_js, MSTR_VFS_LFS_BLOCKDEV_IOCTL);
  jerry_value_t op_js = jerry_create_number(op);
  jerry_value_t arg_js = jerry_create_number(arg);
  jerry_value_t args[2] = {op_js, arg_js};
  jerry_value_t ret = jerry_call_function(ioctl_js, blockdev_js, args, 2);
  int ret_value = 0;
  if (jerry_value_is_number(ret)) {
    ret_value = (int)jerry_get_number_value(ret);
  }
  jerry_release_value(ret);
  jerry_release_value(arg_js);
  jerry_release_value(op_js);
  jerry_release_value(ioctl_js);
  km_tty_printf("bd_ioctl() ->  %d\r\n", ret_value);
  return ret_value;
}

static int bd_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
                   void *buffer, lfs_size_t size) {
  vfs_lfs_handle_t *vfs_handle = (vfs_lfs_handle_t *)c->context;
  // call blockdev.read(block, buffer, offset)
  km_tty_printf("bd_read(lfs_config, %d, %d, buffer, %d)\r\n", block, off,
                size);
  jerry_value_t arraybuffer =
      jerry_create_arraybuffer_external(size, (uint8_t *)buffer, NULL);
  jerry_value_t buffer_js = jerry_create_typedarray_for_arraybuffer(
      JERRY_TYPEDARRAY_UINT8, arraybuffer);
  jerry_value_t read_js =
      jerryxx_get_property(vfs_handle->blockdev_js, MSTR_VFS_LFS_BLOCKDEV_READ);
  jerry_value_t block_js = jerry_create_number(block);
  jerry_value_t offset_js = jerry_create_number(off);
  jerry_value_t args[3] = {block_js, buffer_js, offset_js};
  jerry_value_t ret =
      jerry_call_function(read_js, vfs_handle->blockdev_js, args, 3);
  jerry_release_value(ret);
  jerry_release_value(offset_js);
  jerry_release_value(block_js);
  jerry_release_value(read_js);
  jerry_release_value(buffer_js);
  jerry_release_value(arraybuffer);
  return 0;
}

static int bd_prog(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
                   const void *buffer, lfs_size_t size) {
  vfs_lfs_handle_t *vfs_handle = (vfs_lfs_handle_t *)c->context;
  // call blockdev.write(block, buffer, offset)
  km_tty_printf("bd_prog(lfs_config, %d, %d, buffer, %d)\r\n", block, off,
                size);
  jerry_value_t arraybuffer =
      jerry_create_arraybuffer_external(size, (uint8_t *)buffer, NULL);
  jerry_value_t buffer_js = jerry_create_typedarray_for_arraybuffer(
      JERRY_TYPEDARRAY_UINT8, arraybuffer);
  jerry_value_t write_js = jerryxx_get_property(vfs_handle->blockdev_js,
                                                MSTR_VFS_LFS_BLOCKDEV_WRITE);
  jerry_value_t block_js = jerry_create_number(block);
  jerry_value_t offset_js = jerry_create_number(off);
  jerry_value_t args[3] = {block_js, buffer_js, offset_js};
  jerry_value_t ret =
      jerry_call_function(write_js, vfs_handle->blockdev_js, args, 3);
  jerry_release_value(ret);
  jerry_release_value(offset_js);
  jerry_release_value(block_js);
  jerry_release_value(write_js);
  jerry_release_value(buffer_js);
  jerry_release_value(arraybuffer);
  return 0;
}

static int bd_erase(const struct lfs_config *c, lfs_block_t block) {
  vfs_lfs_handle_t *vfs_handle = (vfs_lfs_handle_t *)c->context;
  km_tty_printf("bd_erase(lfs_config, %d)\r\n", block);
  bd_ioctl(vfs_handle->blockdev_js, 6, block);
  return 0;
}

static int bd_sync(const struct lfs_config *c) {
  vfs_lfs_handle_t *vfs_handle = (vfs_lfs_handle_t *)c->context;
  km_tty_printf("bd_sync(lfs_config)\r\n");
  bd_ioctl(vfs_handle->blockdev_js, 3, 0);
  return 0;
}

/**
 * VFSLittleFS constructor
 * args:
 *   blockdev {object}
 */
JERRYXX_FUN(vfslfs_ctor_fn) {
  // check args
  JERRYXX_CHECK_ARG_OBJECT(0, "blockdev")
  // get args
  jerry_value_t blockdev = JERRYXX_GET_ARG(0);
  // init vfs native handle
  vfs_lfs_handle_t *vfs_handle =
      (vfs_lfs_handle_t *)malloc(sizeof(vfs_lfs_handle_t));
  vfs_lfs_handle_add(vfs_handle);
  vfs_handle->blockdev_js = blockdev;
  jerry_acquire_value(vfs_handle->blockdev_js);

  // lfs config
  vfs_handle->config.context = vfs_handle;
  vfs_handle->config.read = bd_read;
  vfs_handle->config.prog = bd_prog;
  vfs_handle->config.erase = bd_erase;
  vfs_handle->config.sync = bd_sync;
  const int unit_size = 16;
  int block_count = bd_ioctl(vfs_handle->blockdev_js, 4, 0);
  int block_size = bd_ioctl(vfs_handle->blockdev_js, 5, 0);
  vfs_handle->config.read_size = unit_size;
  vfs_handle->config.prog_size = unit_size;
  vfs_handle->config.block_size = block_size;
  vfs_handle->config.block_count = block_count;
  vfs_handle->config.cache_size = unit_size;
  vfs_handle->config.lookahead_size = unit_size;
  vfs_handle->config.block_cycles = 500;
  vfs_handle->config.read_buffer = malloc(vfs_handle->config.cache_size);
  vfs_handle->config.prog_buffer = malloc(vfs_handle->config.cache_size);
  vfs_handle->config.lookahead_buffer =
      malloc(vfs_handle->config.lookahead_size);
  jerry_set_object_native_pointer(this_val, vfs_handle, &vfs_handle_info);

  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.mount()
 */
JERRYXX_FUN(vfs_lfs_mount_fn) {
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);
  km_tty_printf("VFSLittleFS.mount()\r\n");
  int ret = lfs_mount(&vfs_handle->lfs, &vfs_handle->config);
  km_tty_printf("ret=%d", ret);
  if (ret) {
    lfs_format(&vfs_handle->lfs, &vfs_handle->config);
    ret = lfs_mount(&vfs_handle->lfs, &vfs_handle->config);
    if (ret < 0) {
      return jerry_create_error_from_value(create_system_error(ret), true);
    }
  }
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.unmount()
 */
JERRYXX_FUN(vfs_lfs_unmount_fn) {
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);
  km_tty_printf("VFSLittleFS.unmount()");
  int ret = lfs_unmount(&vfs_handle->lfs);
  if (ret < 0) {
    return jerry_create_error_from_value(create_system_error(ret), true);
  }
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.open()
 * args:
 *   path {string}
 *   flags {number} See enum vfslfs_open_flags
 *   mode {number}
 * returns {number} - id
 */
JERRYXX_FUN(vfs_lfs_open_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_CHECK_ARG_NUMBER(1, "flags")
  JERRYXX_CHECK_ARG_NUMBER(2, "mode")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  int flags = (int)JERRYXX_GET_ARG_NUMBER(1);
  int mode = (int)JERRYXX_GET_ARG_NUMBER(2);
  km_tty_printf("VFSLittleFS.open('%s', %d, %d)\r\n", path, flags, mode);
  int lfs_flags = 0;
  if (flags & VFS_FLAG_READ) lfs_flags |= LFS_O_RDONLY;
  if (flags & VFS_FLAG_WRITE) lfs_flags |= LFS_O_WRONLY;
  if (flags & VFS_FLAG_CREATE) lfs_flags |= LFS_O_CREAT;
  if (flags & VFS_FLAG_APPEND) lfs_flags |= LFS_O_APPEND;
  if (flags & VFS_FLAG_EXCL) lfs_flags |= LFS_O_EXCL;
  if (flags & VFS_FLAG_TRUNC) lfs_flags |= LFS_O_TRUNC;
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);
  vfs_lfs_file_handle_t *file = malloc(sizeof(vfs_lfs_file_handle_t));
  int ret = lfs_file_open(&vfs_handle->lfs, &file->lfs_file, path, lfs_flags);
  // ...
  km_tty_printf("VFSLittleFS.open() -> %d\r\n", ret);
  if (ret >= 0) {
    vfs_lfs_file_add(vfs_handle, file);
    uint32_t id = file->id;
    return jerry_create_number(id);
  }
  // TODO: handle 'ret' error code
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.write()
 * args:
 *   id {number}
 *   buffer {TypedArray}
 *   offset {number}
 *   length {number}
 *   position {number}
 * returns {number} - number of bytes written
 */
JERRYXX_FUN(vfs_lfs_write_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "id")
  JERRYXX_CHECK_ARG_OBJECT(1, "buffer")
  JERRYXX_CHECK_ARG_NUMBER(2, "offset")
  JERRYXX_CHECK_ARG_NUMBER(3, "length")
  JERRYXX_CHECK_ARG_NUMBER_OPT(4, "position")
  uint32_t id = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t buffer = JERRYXX_GET_ARG(1);
  if (!jerry_value_is_typedarray(buffer)) {
    // TODO: Exception required for typedarray
  }
  jerry_length_t byteLength = 0;
  jerry_length_t byteOffset = 0;
  jerry_value_t arrbuf =
      jerry_get_typedarray_buffer(buffer, &byteOffset, &byteLength);
  uint8_t *buffer_p = jerry_get_arraybuffer_pointer(arrbuf);
  jerry_release_value(arrbuf);
  uint32_t offset = (uint32_t)JERRYXX_GET_ARG_NUMBER(2);
  uint32_t length = (uint32_t)JERRYXX_GET_ARG_NUMBER(3);
  uint32_t position = (uint32_t)JERRYXX_GET_ARG_NUMBER_OPT(4, 0);
  // TODO: handle offset
  // TODO: handle position
  km_tty_printf("VFSLittleFS.write(%d, buffer, %d, %d, %d)\r\n", id, offset,
                length, position);
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);
  vfs_lfs_file_handle_t *file = vfs_lfs_file_get_by_id(vfs_handle, id);
  int ret = lfs_file_write(&vfs_handle->lfs, &file->lfs_file, (void *)buffer_p,
                           length);
  km_tty_printf("VFSLittleFS.write():byteLength -> %d\r\n", byteLength);
  km_tty_printf("VFSLittleFS.write():byteOffset -> %d\r\n", byteOffset);
  km_tty_printf("VFSLittleFS.write() -> %d\r\n", ret);
  if (ret >= 0) {
    return jerry_create_number(ret);  // bytes written
  }
  // TODO: handle 'ret' error code
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.read()
 * args:
 *   id {number}
 *   buffer {TypedArray}
 *   offset {number}
 *   length {number}
 *   position {number}
 * returns {number} - number of bytes read
 */
JERRYXX_FUN(vfs_lfs_read_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "id")
  JERRYXX_CHECK_ARG_OBJECT(1, "buffer")
  JERRYXX_CHECK_ARG_NUMBER(2, "offset")
  JERRYXX_CHECK_ARG_NUMBER(3, "length")
  JERRYXX_CHECK_ARG_NUMBER(4, "position")
  uint32_t id = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  // TODO: get buffer
  uint32_t offset = (uint32_t)JERRYXX_GET_ARG_NUMBER(2);
  uint32_t length = (uint32_t)JERRYXX_GET_ARG_NUMBER(3);
  uint32_t position = (uint32_t)JERRYXX_GET_ARG_NUMBER(4);
  km_tty_printf("VFSLittleFS.prototype.read(%d, buffer, %d, %d, %d)\r\n", id,
                offset, length, position);
  uint32_t bytes_read = 0;
  // JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfslfs_handle_t, vfs_handle_info);
  return jerry_create_number(bytes_read);
}

/**
 * VFSLittleFS.prototype.close()
 * args:
 *   id {number}
 */
JERRYXX_FUN(vfs_lfs_close_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "id")
  uint32_t id = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  km_tty_printf("VFSLittleFS.close(%d)\r\n", id);
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);
  vfs_lfs_file_handle_t *file = vfs_lfs_file_get_by_id(vfs_handle, id);
  if (file != NULL) {
    int ret = lfs_file_close(&vfs_handle->lfs, &file->lfs_file);
    km_tty_printf("VFSLittleFS.close() -> %d\r\n", ret);
    vfs_lfs_file_remove(vfs_handle, file);
    free(file);
    // TODO: handle 'ret' error code
  }
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.fstat()
 * args:
 *   id {number}
 * returns {fs.Stats}
 */
JERRYXX_FUN(vfs_lfs_fstat_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "id")
  uint32_t id = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  // JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfslfs_handle_t, vfs_handle_info);
  km_tty_printf("VFSLittleFS.fstat(%d)\r\n", id);
  // TODO: return fs.Stat type
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.rename()
 * args:
 *   oldPath {string}
 *   newPath {string}
 */
JERRYXX_FUN(vfs_lfs_rename_fn) {
  JERRYXX_CHECK_ARG_STRING_OPT(0, "oldPath")
  JERRYXX_CHECK_ARG_STRING_OPT(1, "newPath")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, old_path)
  JERRYXX_GET_ARG_STRING_AS_CHAR(1, new_path)
  // JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfslfs_handle_t, vfs_handle_info);
  km_tty_printf("VFSLittleFS.rename('%s', '%s')\r\n", old_path, new_path);
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.unlink()
 * args:
 *   path {string}
 */
JERRYXX_FUN(vfs_lfs_unlink_fn) {
  JERRYXX_CHECK_ARG_STRING_OPT(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  // JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfslfs_handle_t, vfs_handle_info);
  km_tty_printf("VFSLittleFS.unlink('%s')\r\n", path);
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.mkdir()
 * args:
 *   path {string}
 *   mode {number}
 */
JERRYXX_FUN(vfs_lfs_mkdir_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_CHECK_ARG_NUMBER(1, "mode")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  uint32_t mode = (uint32_t)JERRYXX_GET_ARG_NUMBER(1);
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);
  km_tty_printf("VFSLittleFS.mkdir('%s', %d)\r\n", path, mode);
  int ret = lfs_mkdir(&vfs_handle->lfs, path);
  km_tty_printf("VFSLittleFS.mkdir() -> %d\r\n", ret);
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.readdir()
 * args:
 *   path {string}
 * returns {string[]} - array of filenames
 */
JERRYXX_FUN(vfs_lfs_readdir_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);
  km_tty_printf("VFSLittleFS.readdir('%s')\r\n", path);
  lfs_dir_t dir;
  struct lfs_info info;
  int ret = lfs_dir_open(&vfs_handle->lfs, &dir, path);
  if (ret < 0) {
    km_tty_printf("VFSLittleFS.readdir() -> %d\r\n", ret);
    return jerry_create_undefined();
  }
  jerry_value_t files = jerry_create_array(0);
  while (true) {
    int ret = lfs_dir_read(&vfs_handle->lfs, &dir, &info);
    if (ret == 0) {
      break;
    }
    km_tty_printf("-- ENTRY : %s (type=%d, size=%d)\r\n", info.name, info.type,
                  info.size);
    // jerryxx_array_push_string(files, info.name);
  }
  lfs_dir_close(&vfs_handle->lfs, &dir);
  return files;
}

/**
 * VFSLittleFS.prototype.rmdir()
 * args:
 *   path {string}
 */
JERRYXX_FUN(vfs_lfs_rmdir_fn) {
  JERRYXX_CHECK_ARG_STRING_OPT(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  // JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfslfs_handle_t, vfs_handle_info);
  km_tty_printf("VFSLittleFS.rmdir('%s')\r\n", path);
  return jerry_create_undefined();
}

/**
 * Initialize fs_native object and return exportsio
 */
jerry_value_t module_vfs_lfs_init() {
  /* VFSLittleFS class */
  jerry_value_t vfs_lfs_ctor = jerry_create_external_function(vfslfs_ctor_fn);
  jerry_value_t vfs_lfs_prototype = jerry_create_object();
  jerryxx_set_property(vfs_lfs_ctor, "prototype", vfs_lfs_prototype);
  jerryxx_set_property_function(vfs_lfs_prototype, MSTR_VFS_LFS_MOUNT,
                                vfs_lfs_mount_fn);
  jerryxx_set_property_function(vfs_lfs_prototype, MSTR_VFS_LFS_UNMOUNT,
                                vfs_lfs_unmount_fn);
  jerryxx_set_property_function(vfs_lfs_prototype, MSTR_VFS_LFS_OPEN,
                                vfs_lfs_open_fn);
  jerryxx_set_property_function(vfs_lfs_prototype, MSTR_VFS_LFS_WRITE,
                                vfs_lfs_write_fn);
  jerryxx_set_property_function(vfs_lfs_prototype, MSTR_VFS_LFS_READ,
                                vfs_lfs_read_fn);
  jerryxx_set_property_function(vfs_lfs_prototype, MSTR_VFS_LFS_CLOSE,
                                vfs_lfs_close_fn);
  jerryxx_set_property_function(vfs_lfs_prototype, MSTR_VFS_LFS_FSTAT,
                                vfs_lfs_fstat_fn);
  jerryxx_set_property_function(vfs_lfs_prototype, MSTR_VFS_LFS_RENAME,
                                vfs_lfs_rename_fn);
  jerryxx_set_property_function(vfs_lfs_prototype, MSTR_VFS_LFS_UNLINK,
                                vfs_lfs_unlink_fn);
  jerryxx_set_property_function(vfs_lfs_prototype, MSTR_VFS_LFS_MKDIR,
                                vfs_lfs_mkdir_fn);
  jerryxx_set_property_function(vfs_lfs_prototype, MSTR_VFS_LFS_READDIR,
                                vfs_lfs_readdir_fn);
  jerryxx_set_property_function(vfs_lfs_prototype, MSTR_VFS_LFS_RMDIR,
                                vfs_lfs_rmdir_fn);
  jerry_release_value(vfs_lfs_prototype);

  /* vfslittlefs module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property(exports, MSTR_VFS_LFS_VFSLITTLEFS, vfs_lfs_ctor);
  jerry_release_value(vfs_lfs_ctor);

  return exports;
}
