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

#include "fs_magic_strings.h"
#include "io.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "tty.h"  // for tty_printf()

/**
 * fs_native.closeSync()
 * args:
 *   fd {number}
 */
JERRYXX_FUN(fs_close_sync_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "fd")
  uint32_t fd = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  km_tty_printf("fs_native.closeSync(%d)\r\n", fd);
  return jerry_create_undefined();
}

/**
 * fs_native.createReadStream()
 * args:
 *   path {string}
 *   more...
 * returns {fs.ReadStream}
 */
JERRYXX_FUN(fs_create_read_stream_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  // JERRYXX_CHECK_ARG_OBJECT_OPT(1, "options")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  // jerry_value_t options = JERRYXX_GET_ARG(1);
  // TODO: options object should be replaced by detailed mandatory parameters,
  // and the default options should be passed in fs.js file
  km_tty_printf("fs_native.createReadStream('%s', ...)\r\n", path);
  // TODO: return fs.ReadStream type
  return jerry_create_undefined();
}

/**
 * fs_native.createWriteStream()
 * args:
 *   path {string}
 *   more...
 * returns {fs.WriteStream}
 */
JERRYXX_FUN(fs_create_write_stream_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  // JERRYXX_CHECK_ARG_OBJECT_OPT(1, "options")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  // jerry_value_t options = JERRYXX_GET_ARG(1);
  // TODO: options object should be replaced by detailed mandatory parameters,
  // and the default options should be passed in fs.js file
  km_tty_printf("fs_native.createWriteStream('%s', ...)\r\n", path);
  // TODO: return fs.WriteStream type
  return jerry_create_undefined();
}

/**
 * fs_native.existsSync()
 * args:
 *   path {string}
 * returns {boolean}
 */
JERRYXX_FUN(fs_exists_sync_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  km_tty_printf("fs_native.existsSync('%s')\r\n", path);
  bool exists = true;
  return jerry_create_boolean(exists);
}

/**
 * fs_native.fstatSync()
 * args:
 *   fd {number}
 * returns {fs.Stats}
 */
JERRYXX_FUN(fs_fstat_sync_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "fd")
  uint32_t fd = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  km_tty_printf("fs_native.fstatSync(%d)\r\n", fd);
  // TODO: return fs.Stat type
  return jerry_create_undefined();
}

/**
 * fs_native.mkdirSync()
 * args:
 *   path {string}
 *   mode {number}
 */
JERRYXX_FUN(fs_mkdir_sync_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_CHECK_ARG_NUMBER(1, "mode")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  uint32_t mode = (uint32_t)JERRYXX_GET_ARG_NUMBER(1);
  km_tty_printf("fs_native.mkdirSync('%s', %d)\r\n", path, mode);
  return jerry_create_undefined();
}

/**
 * fs_native.openSync()
 * args:
 *   path {string}
 *   flags {string}
 *   mode {number}
 * returns {number} - fd
 */
JERRYXX_FUN(fs_open_sync_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_CHECK_ARG_STRING(1, "flags")
  JERRYXX_CHECK_ARG_NUMBER(2, "mode")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  JERRYXX_GET_ARG_STRING_AS_CHAR(1, flags)
  uint32_t mode = (uint32_t)JERRYXX_GET_ARG_NUMBER(2);
  km_tty_printf("fs_native.openSync('%s', '%s', %d)\r\n", path, flags, mode);
  uint32_t fd = 0;
  return jerry_create_number(fd);
}

/**
 * fs_native.readSync()
 * args:
 *   fd {number}
 *   buffer {TypedArray}
 *   offset {number}
 *   length {number}
 *   position {number}
 * returns {number} - number of bytes read
 */
JERRYXX_FUN(fs_read_sync_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "fd")
  JERRYXX_CHECK_ARG_OBJECT(1, "buffer")
  JERRYXX_CHECK_ARG_NUMBER(2, "offset")
  JERRYXX_CHECK_ARG_NUMBER(3, "length")
  JERRYXX_CHECK_ARG_NUMBER(4, "position")
  uint32_t fd = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  // TODO: get buffer
  uint32_t offset = (uint32_t)JERRYXX_GET_ARG_NUMBER(2);
  uint32_t length = (uint32_t)JERRYXX_GET_ARG_NUMBER(3);
  uint32_t position = (uint32_t)JERRYXX_GET_ARG_NUMBER(4);
  km_tty_printf("fs_native.readSync(%d, buffer, %d, %d, %d)\r\n", fd, offset,
                length, position);
  uint32_t bytes_read = 0;
  return jerry_create_number(bytes_read);
}

/**
 * fs_native.readdirSync()
 * args:
 *   path {string}
 * returns {string[]} - array of filenames
 */
JERRYXX_FUN(fs_readdir_sync_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  km_tty_printf("fs_native.readdirSync('%s')\r\n", path);
  // TODO: return array of strings
  return jerry_create_undefined();
}

/**
 * fs_native.readFileSync()
 * args:
 *   path {string}
 * returns {typedarray?|string?} - content of file
 */
JERRYXX_FUN(fs_read_file_sync_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  km_tty_printf("fs_native.readFileSync('%s')\r\n", path);
  // TODO: return content of file
  return jerry_create_undefined();
}

/**
 * fs_native.renameSync()
 * args:
 *   oldPath {string}
 *   newPath {string}
 */
JERRYXX_FUN(fs_rename_sync_fn) {
  JERRYXX_CHECK_ARG_STRING_OPT(0, "oldPath")
  JERRYXX_CHECK_ARG_STRING_OPT(1, "newPath")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, old_path)
  JERRYXX_GET_ARG_STRING_AS_CHAR(1, new_path)
  km_tty_printf("fs_native.renameSync('%s', '%s')\r\n", old_path, new_path);
  return jerry_create_undefined();
}

/**
 * fs_native.rmdirSync()
 * args:
 *   path {string}
 */
JERRYXX_FUN(fs_rmdir_sync_fn) {
  JERRYXX_CHECK_ARG_STRING_OPT(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  km_tty_printf("fs_native.rmdirSync('%s')\r\n", path);
  return jerry_create_undefined();
}

/**
 * fs_native.statSync()
 * args:
 *   path {string}
 * returns {fs.Stats}
 */
JERRYXX_FUN(fs_stat_sync_fn) {
  JERRYXX_CHECK_ARG_STRING_OPT(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  km_tty_printf("fs_native.statSync('%s')\r\n", path);
  // TODO: return fs.Stat type
  return jerry_create_undefined();
}

/**
 * fs_native.unlinkSync()
 * args:
 *   path {string}
 */
JERRYXX_FUN(fs_unlink_sync_fn) {
  JERRYXX_CHECK_ARG_STRING_OPT(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  km_tty_printf("fs_native.unlinkSync('%s')\r\n", path);
  return jerry_create_undefined();
}

/**
 * fs_native.writeSync()
 * args:
 *   fd {number}
 *   buffer {TypedArray}
 *   offset {number}
 *   length {number}
 *   position {number}
 * returns {number} - number of bytes written
 */
JERRYXX_FUN(fs_write_sync_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "fd")
  JERRYXX_CHECK_ARG_OBJECT(1, "buffer")
  JERRYXX_CHECK_ARG_NUMBER(2, "offset")
  JERRYXX_CHECK_ARG_NUMBER(3, "length")
  JERRYXX_CHECK_ARG_NUMBER(4, "position")
  uint32_t fd = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  // TODO: get buffer
  uint32_t offset = (uint32_t)JERRYXX_GET_ARG_NUMBER(2);
  uint32_t length = (uint32_t)JERRYXX_GET_ARG_NUMBER(3);
  uint32_t position = (uint32_t)JERRYXX_GET_ARG_NUMBER(4);
  km_tty_printf("fs_native.writeSync(%d, buffer, %d, %d, %d)\r\n", fd, offset,
                length, position);
  uint32_t bytes_written = 0;
  return jerry_create_number(bytes_written);
}

/**
 * fs_native.writeFileSync()
 * args:
 *   path {string}
 *   data {string|typedarray}
 */
JERRYXX_FUN(fs_write_file_sync_fn) {
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_CHECK_ARG_OBJECT(1, "data")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  km_tty_printf("fs_native.writeFileSync('%s', data)\r\n", path);
  return jerry_create_undefined();
}

/**
 * Initialize fs_native object and return exports
 */
jerry_value_t module_fs_init() {
  /* fs_native exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property_function(exports, MSTR_FS_CLOSE_SYNC, fs_close_sync_fn);
  jerryxx_set_property_function(exports, MSTR_FS_CREATE_READ_STREAM,
                                fs_create_read_stream_fn);
  jerryxx_set_property_function(exports, MSTR_FS_CREATE_WRITE_STREAM,
                                fs_create_write_stream_fn);
  jerryxx_set_property_function(exports, MSTR_FS_EXISTS_SYNC,
                                fs_exists_sync_fn);
  jerryxx_set_property_function(exports, MSTR_FS_FSTAT_SYNC, fs_fstat_sync_fn);
  jerryxx_set_property_function(exports, MSTR_FS_MKDIR_SYNC, fs_mkdir_sync_fn);
  jerryxx_set_property_function(exports, MSTR_FS_OPEN_SYNC, fs_open_sync_fn);
  jerryxx_set_property_function(exports, MSTR_FS_READ_SYNC, fs_read_sync_fn);
  jerryxx_set_property_function(exports, MSTR_FS_READDIR_SYNC,
                                fs_readdir_sync_fn);
  jerryxx_set_property_function(exports, MSTR_FS_READ_FILE_SYNC,
                                fs_read_file_sync_fn);
  jerryxx_set_property_function(exports, MSTR_FS_RENAME_SYNC,
                                fs_rename_sync_fn);
  jerryxx_set_property_function(exports, MSTR_FS_RMDIR_SYNC, fs_rmdir_sync_fn);
  jerryxx_set_property_function(exports, MSTR_FS_STAT_SYNC, fs_stat_sync_fn);
  jerryxx_set_property_function(exports, MSTR_FS_UNLINK_SYNC,
                                fs_unlink_sync_fn);
  jerryxx_set_property_function(exports, MSTR_FS_WRITE_SYNC, fs_write_sync_fn);
  jerryxx_set_property_function(exports, MSTR_FS_WRITE_FILE_SYNC,
                                fs_write_file_sync_fn);
  return exports;
}
