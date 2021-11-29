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

#include "module_vfslfs.h"

#include <stdlib.h>

#include "io.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "lfs.h"
#include "tty.h"  // for tty_printf()
#include "vfslfs.h"
#include "vfslfs_magic_strings.h"

static void vfs_handle_freecb(void *handle) {
  vfslfs_handle_t *vfs_handle = (vfslfs_handle_t *)handle;
  jerry_release_value(vfs_handle->blockdev_js);
  free(handle);
}

static const jerry_object_native_info_t vfs_handle_info = {
    .free_cb = vfs_handle_freecb};

static int bd_read(const struct lfs_config *c, lfs_block_t block, lfs_off_t off,
                   void *buffer, lfs_size_t size) {
  vfslfs_handle_t *vfs_handle = (vfslfs_handle_t *)c->context;
  // call blockdev.read(block, buffer, offset)
  km_tty_printf("bd_read(lfs_config, %d, %d, buffer, %d)\r\n", block, off,
                size);
  jerry_value_t arraybuffer =
      jerry_create_arraybuffer_external(size, (uint8_t *)buffer, NULL);
  jerry_value_t buffer_js = jerry_create_typedarray_for_arraybuffer(
      JERRY_TYPEDARRAY_UINT8, arraybuffer);
  jerry_value_t read_js =
      jerryxx_get_property(vfs_handle->blockdev_js, MSTR_VFSLFS_BLOCKDEV_READ);
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
  vfslfs_handle_t *vfs_handle = (vfslfs_handle_t *)c->context;
  // call blockdev.write(block, buffer, offset)
  km_tty_printf("bd_prog(lfs_config, %d, %d, buffer, %d)\r\n", block, off,
                size);
  jerry_value_t arraybuffer =
      jerry_create_arraybuffer_external(size, (uint8_t *)buffer, NULL);
  jerry_value_t buffer_js = jerry_create_typedarray_for_arraybuffer(
      JERRY_TYPEDARRAY_UINT8, arraybuffer);
  jerry_value_t write_js =
      jerryxx_get_property(vfs_handle->blockdev_js, MSTR_VFSLFS_BLOCKDEV_WRITE);
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
  // vfslfs_handle_t *vfs_handle = (vfslfs_handle_t *)c->context;
  // call vfs_handle->blockdev_js's ioctl(6, block)
  km_tty_printf("bd_erase(lfs_config, %d)\r\n", block);
  return 0;
}

static int bd_sync(const struct lfs_config *c) {
  // vfslfs_handle_t *vfs_handle = (vfslfs_handle_t *)c->context;
  // call vfs_handle->blockdev_js's sync(3)
  km_tty_printf("bd_sync(lfs_config)\r\n");
  return 0;
}

static int bd_ioctl(jerry_value_t blockdev_js, int op, int arg) {
  km_tty_printf("bd_ioctl(%d, %d)\r\n", op, arg);
  jerry_value_t ioctl_js =
      jerryxx_get_property(blockdev_js, MSTR_VFSLFS_BLOCKDEV_IOCTL);
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
  vfslfs_handle_t *vfs_handle =
      (vfslfs_handle_t *)malloc(sizeof(vfslfs_handle_t));
  vfs_handle->blockdev_js = blockdev;
  jerry_acquire_value(vfs_handle->blockdev_js);

  // lfs config
  vfs_handle->config.context = vfs_handle;
  vfs_handle->config.read = bd_read;
  vfs_handle->config.prog = bd_prog;
  vfs_handle->config.erase = bd_erase;
  vfs_handle->config.sync = bd_sync;
  // TODO: set correct config values
  const int unit_size = 16;
  int block_count = bd_ioctl(vfs_handle->blockdev_js, 4, 0);
  int block_size = bd_ioctl(vfs_handle->blockdev_js, 5, 0);

  km_tty_printf("blocksize=%d\r\n", block_size);
  km_tty_printf("blockcount=%d\r\n", block_count);

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
JERRYXX_FUN(vfslfs_mount_fn) {
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfslfs_handle_t, vfs_handle_info);
  km_tty_printf("VFSLittleFS.mount()\r\n");
  km_tty_printf("config.read_size = %d\r\n", vfs_handle->config.read_size);
  km_tty_printf("config.block_size = %d\r\n", vfs_handle->config.block_size);
  km_tty_printf("config.block_count = %d\r\n", vfs_handle->config.block_count);

  int err = lfs_mount(&vfs_handle->lfs, &vfs_handle->config);
  km_tty_printf("ret=%d", err);
  if (err) {
    lfs_format(&vfs_handle->lfs, &vfs_handle->config);
    lfs_mount(&vfs_handle->lfs, &vfs_handle->config);
  }
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.unmount()
 */
JERRYXX_FUN(vfslfs_unmount_fn) {
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfslfs_handle_t, vfs_handle_info);
  km_tty_printf("VFSLittleFS.unmount()");
  lfs_unmount(&vfs_handle->lfs);
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.open()
 * args:
 *   path {string}
 *   flags {string}
 *   mode {number}
 * returns {number} - id
 */
JERRYXX_FUN(vfslfs_open_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_CHECK_ARG_STRING(1, "flags")
  JERRYXX_CHECK_ARG_NUMBER(2, "mode")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  JERRYXX_GET_ARG_STRING_AS_CHAR(1, flags)
  uint32_t mode = (uint32_t)JERRYXX_GET_ARG_NUMBER(2);
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfslfs_handle_t, vfs_handle_info);
  km_tty_printf("VFSLittleFS.open('%s', '%s', %d)\r\n", path, flags, mode);
  vfslfs_file_handle_t *file = malloc(sizeof(vfslfs_file_handle_t));
  // int ret = lfs_file_open(&vfs_handle->lfs, &file->lfs_file, "", 0);
  // ...
  vfslfs_file_add(vfs_handle, file);
  uint32_t id = file->id;
  return jerry_create_number(id);
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
JERRYXX_FUN(vfslfs_write_fn) {
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
  km_tty_printf("VFSLittleFS.write(%d, buffer, %d, %d, %d)\r\n", id, offset,
                length, position);
  uint32_t bytes_written = 0;
  // JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfslfs_handle_t, vfs_handle_info);
  return jerry_create_number(bytes_written);
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
JERRYXX_FUN(vfslfs_read_fn) {
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
JERRYXX_FUN(vfslfs_close_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "id")
  uint32_t id = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  // JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfslfs_handle_t, vfs_handle_info);
  km_tty_printf("VFSLittleFS.close(%d)\r\n", id);
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.fstat()
 * args:
 *   id {number}
 * returns {fs.Stats}
 */
JERRYXX_FUN(vfslfs_fstat_fn) {
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
JERRYXX_FUN(vfslfs_rename_fn) {
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
JERRYXX_FUN(vfslfs_unlink_fn) {
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
JERRYXX_FUN(vfslfs_mkdir_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_CHECK_ARG_NUMBER(1, "mode")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  uint32_t mode = (uint32_t)JERRYXX_GET_ARG_NUMBER(1);
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfslfs_handle_t, vfs_handle_info);
  km_tty_printf("VFSLittleFS.mkdir('%s', %d)\r\n", path, mode);
  lfs_mkdir(&vfs_handle->lfs, path);
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.readdir()
 * args:
 *   path {string}
 * returns {string[]} - array of filenames
 */
JERRYXX_FUN(vfslfs_readdir_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfslfs_handle_t, vfs_handle_info);
  km_tty_printf("VFSLittleFS.readdir('%s')\r\n", path);
  lfs_dir_t dir;
  struct lfs_info info;
  int err = lfs_dir_open(&vfs_handle->lfs, &dir, path);
  if (err) {
    return err;
  }
  jerry_value_t files = jerry_create_array(0);
  while (true) {
    int ret = lfs_dir_read(&vfs_handle->lfs, &dir, &info);
    if (ret == 0) {
      break;
    }
    km_tty_printf("%s\r\n", info.name);
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
JERRYXX_FUN(vfslfs_rmdir_fn) {
  JERRYXX_CHECK_ARG_STRING_OPT(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  // JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfslfs_handle_t, vfs_handle_info);
  km_tty_printf("VFSLittleFS.rmdir('%s')\r\n", path);
  return jerry_create_undefined();
}

/**
 * Initialize fs_native object and return exports
 */
jerry_value_t module_vfslfs_init() {
  /* VFSLittleFS class */
  jerry_value_t vfslfs_ctor = jerry_create_external_function(vfslfs_ctor_fn);
  jerry_value_t vfslfs_prototype = jerry_create_object();
  jerryxx_set_property(vfslfs_ctor, "prototype", vfslfs_prototype);
  jerryxx_set_property_function(vfslfs_prototype, MSTR_VFSLFS_MOUNT,
                                vfslfs_mount_fn);
  jerryxx_set_property_function(vfslfs_prototype, MSTR_VFSLFS_UNMOUNT,
                                vfslfs_unmount_fn);
  jerryxx_set_property_function(vfslfs_prototype, MSTR_VFSLFS_OPEN,
                                vfslfs_open_fn);
  jerryxx_set_property_function(vfslfs_prototype, MSTR_VFSLFS_WRITE,
                                vfslfs_write_fn);
  jerryxx_set_property_function(vfslfs_prototype, MSTR_VFSLFS_READ,
                                vfslfs_read_fn);
  jerryxx_set_property_function(vfslfs_prototype, MSTR_VFSLFS_CLOSE,
                                vfslfs_close_fn);
  jerryxx_set_property_function(vfslfs_prototype, MSTR_VFSLFS_FSTAT,
                                vfslfs_fstat_fn);
  jerryxx_set_property_function(vfslfs_prototype, MSTR_VFSLFS_RENAME,
                                vfslfs_rename_fn);
  jerryxx_set_property_function(vfslfs_prototype, MSTR_VFSLFS_UNLINK,
                                vfslfs_unlink_fn);
  jerryxx_set_property_function(vfslfs_prototype, MSTR_VFSLFS_MKDIR,
                                vfslfs_mkdir_fn);
  jerryxx_set_property_function(vfslfs_prototype, MSTR_VFSLFS_READDIR,
                                vfslfs_readdir_fn);
  jerryxx_set_property_function(vfslfs_prototype, MSTR_VFSLFS_RMDIR,
                                vfslfs_rmdir_fn);
  jerry_release_value(vfslfs_prototype);

  /* vfslittlefs module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property(exports, MSTR_VFSLFS_VFSLITTLEFS, vfslfs_ctor);
  jerry_release_value(vfslfs_ctor);

  return exports;
}
