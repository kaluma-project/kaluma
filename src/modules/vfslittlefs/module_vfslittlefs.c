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

#include <stdlib.h>

#include "io.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "tty.h"  // for tty_printf()
#include "vfslittlefs_magic_strings.h"

/**
 * VFSLittleFS constructor
 */
JERRYXX_FUN(vfs_littlefs_ctor_fn) { return jerry_create_undefined(); }

/**
 * VFSLittleFS.prototype.mount()
 * args:
 *   blockdev {object}
 */
JERRYXX_FUN(vfs_littlefs_mount_fn) {
  // check args
  JERRYXX_CHECK_ARG_OBJECT(0, "blockdev")

  // get args
  jerry_value_t bdev = JERRYXX_GET_ARG(0);
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.unmount()
 */
JERRYXX_FUN(vfs_littlefs_unmount_fn) { return jerry_create_undefined(); }

/**
 * VFSLittleFS.prototype.open()
 * args:
 *   path {string}
 *   flags {string}
 *   mode {number}
 * returns {number} - fd
 */
JERRYXX_FUN(vfs_littlefs_open_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_CHECK_ARG_STRING(1, "flags")
  JERRYXX_CHECK_ARG_NUMBER(2, "mode")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  JERRYXX_GET_ARG_STRING_AS_CHAR(1, flags)
  uint32_t mode = (uint32_t)JERRYXX_GET_ARG_NUMBER(2);
  km_tty_printf("VFSLittleFS.open('%s', '%s', %d)\r\n", path, flags, mode);
  uint32_t fd = 0;
  return jerry_create_number(fd);
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
JERRYXX_FUN(vfs_littlefs_write_fn) {
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
JERRYXX_FUN(vfs_littlefs_read_fn) {
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
  return jerry_create_number(bytes_read);
}

/**
 * VFSLittleFS.prototype.close()
 * args:
 *   id {number}
 */
JERRYXX_FUN(vfs_littlefs_close_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "id")
  uint32_t id = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  km_tty_printf("VFSLittleFS.close(%d)\r\n", id);
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.fstat()
 * args:
 *   id {number}
 * returns {fs.Stats}
 */
JERRYXX_FUN(vfs_littlefs_fstat_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "id")
  uint32_t id = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
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
JERRYXX_FUN(vfs_littlefs_rename_fn) {
  JERRYXX_CHECK_ARG_STRING_OPT(0, "oldPath")
  JERRYXX_CHECK_ARG_STRING_OPT(1, "newPath")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, old_path)
  JERRYXX_GET_ARG_STRING_AS_CHAR(1, new_path)
  km_tty_printf("VFSLittleFS.rename('%s', '%s')\r\n", old_path, new_path);
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.unlink()
 * args:
 *   path {string}
 */
JERRYXX_FUN(vfs_littlefs_unlink_fn) {
  JERRYXX_CHECK_ARG_STRING_OPT(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  km_tty_printf("VFSLittleFS.unlink('%s')\r\n", path);
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.mkdir()
 * args:
 *   path {string}
 *   mode {number}
 */
JERRYXX_FUN(vfs_littlefs_mkdir_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_CHECK_ARG_NUMBER(1, "mode")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  uint32_t mode = (uint32_t)JERRYXX_GET_ARG_NUMBER(1);
  km_tty_printf("VFSLittleFS.mkdir('%s', %d)\r\n", path, mode);
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.readdir()
 * args:
 *   path {string}
 * returns {string[]} - array of filenames
 */
JERRYXX_FUN(vfs_littlefs_readdir_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  km_tty_printf("VFSLittleFS.readdir('%s')\r\n", path);
  // TODO: return array of strings
  return jerry_create_undefined();
}

/**
 * VFSLittleFS.prototype.rmdir()
 * args:
 *   path {string}
 */
JERRYXX_FUN(vfs_littlefs_rmdir_fn) {
  JERRYXX_CHECK_ARG_STRING_OPT(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  km_tty_printf("VFSLittleFS.rmdir('%s')\r\n", path);
  return jerry_create_undefined();
}

/**
 * Initialize fs_native object and return exports
 */
jerry_value_t module_vfslittlefs_init() {
  /* VFSLittleFS class */
  jerry_value_t vfs_littlefs_ctor =
      jerry_create_external_function(vfs_littlefs_ctor_fn);
  jerry_value_t vfs_littlefs_prototype = jerry_create_object();
  jerryxx_set_property(vfs_littlefs_ctor, "prototype", vfs_littlefs_prototype);
  jerryxx_set_property_function(vfs_littlefs_prototype, MSTR_VFSLITTLEFS_MOUNT,
                                vfs_littlefs_mount_fn);
  jerryxx_set_property_function(vfs_littlefs_prototype,
                                MSTR_VFSLITTLEFS_UNMOUNT,
                                vfs_littlefs_unmount_fn);
  jerryxx_set_property_function(vfs_littlefs_prototype, MSTR_VFSLITTLEFS_OPEN,
                                vfs_littlefs_open_fn);
  jerryxx_set_property_function(vfs_littlefs_prototype, MSTR_VFSLITTLEFS_WRITE,
                                vfs_littlefs_write_fn);
  jerryxx_set_property_function(vfs_littlefs_prototype, MSTR_VFSLITTLEFS_READ,
                                vfs_littlefs_read_fn);
  jerryxx_set_property_function(vfs_littlefs_prototype, MSTR_VFSLITTLEFS_CLOSE,
                                vfs_littlefs_close_fn);
  jerryxx_set_property_function(vfs_littlefs_prototype, MSTR_VFSLITTLEFS_FSTAT,
                                vfs_littlefs_fstat_fn);
  jerryxx_set_property_function(vfs_littlefs_prototype, MSTR_VFSLITTLEFS_RENAME,
                                vfs_littlefs_rename_fn);
  jerryxx_set_property_function(vfs_littlefs_prototype, MSTR_VFSLITTLEFS_UNLINK,
                                vfs_littlefs_unlink_fn);
  jerryxx_set_property_function(vfs_littlefs_prototype, MSTR_VFSLITTLEFS_MKDIR,
                                vfs_littlefs_mkdir_fn);
  jerryxx_set_property_function(vfs_littlefs_prototype,
                                MSTR_VFSLITTLEFS_READDIR,
                                vfs_littlefs_readdir_fn);
  jerryxx_set_property_function(vfs_littlefs_prototype, MSTR_VFSLITTLEFS_RMDIR,
                                vfs_littlefs_rmdir_fn);
  jerry_release_value(vfs_littlefs_prototype);

  /* vfslittlefs module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property(exports, MSTR_VFSLITTLEFS_VFSLITTLEFS,
                       vfs_littlefs_ctor);
  jerry_release_value(vfs_littlefs_ctor);

  return exports;
}
