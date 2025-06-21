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
#include "magic_strings.h"
#include "vfs_lfs.h"
#include "vfs_lfs_magic_strings.h"

static void vfs_handle_freecb(void *handle, struct jerry_object_native_info_t *info_p) {
  vfs_lfs_handle_t *vfs_handle = (vfs_lfs_handle_t *)handle;
  free(vfs_handle->config.lookahead_buffer);
  free(vfs_handle->config.prog_buffer);
  free(vfs_handle->config.read_buffer);
  jerry_value_free(vfs_handle->blkdev_js);
  vfs_lfs_handle_remove(handle);
  free(handle);
}

static const jerry_object_native_info_t vfs_handle_info = {
    .free_cb = vfs_handle_freecb};

static int blkdev_ioctl(jerry_value_t blkdev_js, int op, int arg) {
  // km_tty_printf("blkdev_ioctl(%d, %d)\r\n", op, arg);
  jerry_value_t ioctl_js = jerryxx_get_property(blkdev_js, "ioctl");
  jerry_value_t op_js = jerry_number(op);
  jerry_value_t arg_js = jerry_number(arg);
  jerry_value_t args[2] = {op_js, arg_js};
  jerry_value_t ret = jerry_call(ioctl_js, blkdev_js, args, 2);
  int ret_value = 0;
  if (jerry_value_is_number(ret)) {
    ret_value = (int)jerry_value_as_number(ret);
  }
  jerry_value_free(ret);
  jerry_value_free(arg_js);
  jerry_value_free(op_js);
  jerry_value_free(ioctl_js);
  // km_tty_printf("blkdev_ioctl() ->  %d\r\n", ret_value);
  return ret_value;
}

static int blkdev_read(const struct lfs_config *c, lfs_block_t block,
                       lfs_off_t off, void *buffer, lfs_size_t size) {
  vfs_lfs_handle_t *vfs_handle = (vfs_lfs_handle_t *)c->context;
  // call blockdev.read(block, buffer, offset)
  // km_tty_printf("blkdev_read(lfs_config, %d, %d, buffer, %d)\r\n", block,
  // off, size);
  jerry_value_t arraybuffer =
      jerry_arraybuffer_external((uint8_t *)buffer, size, NULL);
  jerry_value_t buffer_js = jerry_typedarray_with_buffer(
      JERRY_TYPEDARRAY_UINT8, arraybuffer);
  jerry_value_t read_js = jerryxx_get_property(vfs_handle->blkdev_js, "read");
  jerry_value_t block_js = jerry_number(block);
  jerry_value_t offset_js = jerry_number(off);
  jerry_value_t args[3] = {block_js, buffer_js, offset_js};
  jerry_value_t ret =
      jerry_call(read_js, vfs_handle->blkdev_js, args, 3);
  jerry_value_free(ret);
  jerry_value_free(offset_js);
  jerry_value_free(block_js);
  jerry_value_free(read_js);
  jerry_value_free(buffer_js);
  jerry_value_free(arraybuffer);
  return 0;
}

static int blkdev_prog(const struct lfs_config *c, lfs_block_t block,
                       lfs_off_t off, const void *buffer, lfs_size_t size) {
  vfs_lfs_handle_t *vfs_handle = (vfs_lfs_handle_t *)c->context;
  // call blockdev.write(block, buffer, offset)
  // km_tty_printf("blkdev_prog(lfs_config, %d, %d, buffer, %d)\r\n", block,
  // off, size);
  jerry_value_t arraybuffer =
      jerry_arraybuffer_external((uint8_t *)buffer, size, NULL);
  jerry_value_t buffer_js = jerry_typedarray_with_buffer(
      JERRY_TYPEDARRAY_UINT8, arraybuffer);
  jerry_value_t write_js = jerryxx_get_property(vfs_handle->blkdev_js, "write");
  jerry_value_t block_js = jerry_number(block);
  jerry_value_t offset_js = jerry_number(off);
  jerry_value_t args[3] = {block_js, buffer_js, offset_js};
  jerry_value_t ret =
      jerry_call(write_js, vfs_handle->blkdev_js, args, 3);
  jerry_value_free(ret);
  jerry_value_free(offset_js);
  jerry_value_free(block_js);
  jerry_value_free(write_js);
  jerry_value_free(buffer_js);
  jerry_value_free(arraybuffer);
  return 0;
}

static int blkdev_erase(const struct lfs_config *c, lfs_block_t block) {
  vfs_lfs_handle_t *vfs_handle = (vfs_lfs_handle_t *)c->context;
  // km_tty_printf("blkdev_erase(lfs_config, %d)\r\n", block);
  blkdev_ioctl(vfs_handle->blkdev_js, 6, block);
  return 0;
}

static int blkdev_sync(const struct lfs_config *c) {
  vfs_lfs_handle_t *vfs_handle = (vfs_lfs_handle_t *)c->context;
  // km_tty_printf("blkdev_sync(lfs_config)\r\n");
  blkdev_ioctl(vfs_handle->blkdev_js, 3, 0);
  return 0;
}

/**
 * VFSLittleFS constructor
 * args:
 *   blockdev {object}
 */
JERRYXX_FUN(vfslfs_ctor_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_OBJECT(0, "blkdev")
  jerry_value_t blkdev = JERRYXX_GET_ARG(0);

  // initialize vfs native handle
  vfs_lfs_handle_t *vfs_handle =
      (vfs_lfs_handle_t *)malloc(sizeof(vfs_lfs_handle_t));
  vfs_lfs_handle_init(vfs_handle);
  vfs_lfs_handle_add(vfs_handle);
  vfs_handle->blkdev_js = blkdev;
  // jerry_value_copy(vfs_handle->blkdev_js);
  vfs_handle->config.context = vfs_handle;
  vfs_handle->config.read = blkdev_read;
  vfs_handle->config.prog = blkdev_prog;
  vfs_handle->config.erase = blkdev_erase;
  vfs_handle->config.sync = blkdev_sync;
  int block_count = blkdev_ioctl(vfs_handle->blkdev_js, 4, 0);
  int block_size = blkdev_ioctl(vfs_handle->blkdev_js, 5, 0);
  int unit_size = blkdev_ioctl(vfs_handle->blkdev_js, 7, 0);
  vfs_handle->config.read_size = unit_size;
  vfs_handle->config.prog_size = unit_size;
  vfs_handle->config.block_size = block_size;
  vfs_handle->config.block_count = block_count;
  vfs_handle->config.cache_size = unit_size;
  vfs_handle->config.lookahead_size = unit_size;
  vfs_handle->config.name_max = 255;
  vfs_handle->config.file_max = 1024 * 1024 * 16;  // 16MB
  vfs_handle->config.attr_max = 512;
  vfs_handle->config.block_cycles = 500;
  vfs_handle->config.read_buffer = malloc(vfs_handle->config.cache_size);
  vfs_handle->config.prog_buffer = malloc(vfs_handle->config.cache_size);
  vfs_handle->config.lookahead_buffer =
      malloc(vfs_handle->config.lookahead_size);

  // assign native handle in js object
  jerry_object_set_native_ptr(JERRYXX_GET_THIS, &vfs_handle_info, vfs_handle);

  return jerry_undefined();
}

/**
 * VFSLittleFS.prototype.mkfs()
 */
JERRYXX_FUN(vfs_lfs_mkfs_fn) {
  // get native vfs handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);

  // initialize block device
  blkdev_ioctl(vfs_handle->blkdev_js, 1, 0);

  // make fs (format)
  int ret = lfs_format(&vfs_handle->lfs, &vfs_handle->config);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }
  return jerry_undefined();
}

/**
 * VFSLittleFS.prototype.mount()
 */
JERRYXX_FUN(vfs_lfs_mount_fn) {
  // get native vfs handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);

  // initialize block device
  blkdev_ioctl(vfs_handle->blkdev_js, 1, 0);

  // mount vfs
  int ret = lfs_mount(&vfs_handle->lfs, &vfs_handle->config);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }
  return jerry_undefined();
}

/**
 * VFSLittleFS.prototype.unmount()
 */
JERRYXX_FUN(vfs_lfs_unmount_fn) {
  // get native vfs handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);

  // unmount vfs
  int ret = lfs_unmount(&vfs_handle->lfs);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }

  // shutdown block device
  blkdev_ioctl(vfs_handle->blkdev_js, 2, 0);
  return jerry_undefined();
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
  // check and get args
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_CHECK_ARG_NUMBER(1, "flags")
  JERRYXX_CHECK_ARG_NUMBER(2, "mode")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  int flags = (int)JERRYXX_GET_ARG_NUMBER(1);
  // int mode = (int)JERRYXX_GET_ARG_NUMBER(2);

  // convert flags to lfs_flags
  int lfs_flags = 0;
  if (flags & VFS_FLAG_READ) lfs_flags |= LFS_O_RDONLY;
  if (flags & VFS_FLAG_WRITE) lfs_flags |= LFS_O_WRONLY;
  if (flags & VFS_FLAG_CREATE) lfs_flags |= LFS_O_CREAT;
  if (flags & VFS_FLAG_APPEND) lfs_flags |= LFS_O_APPEND;
  if (flags & VFS_FLAG_EXCL) lfs_flags |= LFS_O_EXCL;
  if (flags & VFS_FLAG_TRUNC) lfs_flags |= LFS_O_TRUNC;

  // get native vfs handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);

  // create file handle
  vfs_lfs_file_handle_t *file = malloc(sizeof(vfs_lfs_file_handle_t));

  // file open
  int ret = lfs_file_open(&vfs_handle->lfs, &file->lfs_file, path, lfs_flags);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }

  // add file handle and return the id
  vfs_lfs_file_add(vfs_handle, file);
  uint32_t id = file->id;
  return jerry_number(id);
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
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER(0, "id")
  JERRYXX_CHECK_ARG_TYPEDARRAY(1, "buffer")
  JERRYXX_CHECK_ARG_NUMBER_OPT(2, "offset")
  JERRYXX_CHECK_ARG_NUMBER_OPT(3, "length")
  JERRYXX_CHECK_ARG_NUMBER_OPT(4, "position")
  uint32_t id = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t buffer = JERRYXX_GET_ARG(1);
  jerry_length_t buf_length = 0;
  jerry_length_t buf_offset = 0;
  jerry_value_t arrbuf =
      jerry_typedarray_buffer(buffer, &buf_offset, &buf_length);
  uint8_t *buffer_p = jerry_arraybuffer_data(arrbuf);
  jerry_value_free(arrbuf);
  int offset = (int)JERRYXX_GET_ARG_NUMBER_OPT(2, 0);
  int length = (int)JERRYXX_GET_ARG_NUMBER_OPT(3, buf_length);
  int position = (int)JERRYXX_GET_ARG_NUMBER_OPT(4, -1);

  // get native file handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);
  VFS_LFS_GET_FILE_HANDLE(vfs_handle, file, id)

  // set position
  if (position > -1) {
    int pos = lfs_file_seek(&vfs_handle->lfs, &file->lfs_file, position,
                            LFS_SEEK_SET);
    if (pos < 0) {
      return jerry_exception_value(create_system_error(pos), true);
    }
  }

  // file write
  int ret = lfs_file_write(&vfs_handle->lfs, &file->lfs_file,
                           (void *)(buffer_p + offset), length);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }

  // return number of bytes written
  return jerry_number(ret);
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
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER(0, "id")
  JERRYXX_CHECK_ARG_TYPEDARRAY(1, "buffer")
  JERRYXX_CHECK_ARG_NUMBER_OPT(2, "offset")
  JERRYXX_CHECK_ARG_NUMBER_OPT(3, "length")
  JERRYXX_CHECK_ARG_NUMBER_OPT(4, "position")
  uint32_t id = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);
  jerry_value_t buffer = JERRYXX_GET_ARG(1);
  jerry_length_t buf_length = 0;
  jerry_length_t buf_offset = 0;
  jerry_value_t arrbuf =
      jerry_typedarray_buffer(buffer, &buf_offset, &buf_length);
  uint8_t *buffer_p = jerry_arraybuffer_data(arrbuf);
  jerry_value_free(arrbuf);
  int offset = (int)JERRYXX_GET_ARG_NUMBER_OPT(2, 0);
  int length = (int)JERRYXX_GET_ARG_NUMBER_OPT(3, buf_length);
  int position = (int)JERRYXX_GET_ARG_NUMBER_OPT(4, -1);

  // get native file handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);
  VFS_LFS_GET_FILE_HANDLE(vfs_handle, file, id)

  // set position
  if (position > -1) {
    int pos = lfs_file_seek(&vfs_handle->lfs, &file->lfs_file, position,
                            LFS_SEEK_SET);
    if (pos < 0) {
      return jerry_exception_value(create_system_error(pos), true);
    }
  }

  // file read
  int ret = lfs_file_read(&vfs_handle->lfs, &file->lfs_file,
                          (uint8_t *)(buffer_p + offset), length);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }

  // return number of bytes read
  return jerry_number(ret);
}

/**
 * VFSLittleFS.prototype.close()
 * args:
 *   id {number}
 */
JERRYXX_FUN(vfs_lfs_close_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER(0, "id")
  uint32_t id = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);

  // get native file handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);
  VFS_LFS_GET_FILE_HANDLE(vfs_handle, file, id)

  // file close
  int ret = lfs_file_close(&vfs_handle->lfs, &file->lfs_file);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }

  // remote file handle
  vfs_lfs_file_remove(vfs_handle, file);
  free(file);

  // return
  return jerry_undefined();
}

/**
 * VFSLittleFS.prototype.stat()
 * args:
 *   path {string}
 * returns {fs.Stats}
 */
JERRYXX_FUN(vfs_lfs_stat_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)

  // get native vfs handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);

  // file stat
  struct lfs_info info;
  int ret = lfs_stat(&vfs_handle->lfs, path, &info);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }

  // return stat object {type, size}
  jerry_value_t obj = jerry_object();
  jerryxx_set_property_number(obj, "type", info.type);
  jerryxx_set_property_number(obj, "size",
                              info.type == LFS_TYPE_REG ? info.size : 0);
  return obj;
}

/**
 * VFSLittleFS.prototype.rename()
 * args:
 *   oldPath {string}
 *   newPath {string}
 */
JERRYXX_FUN(vfs_lfs_rename_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_STRING_OPT(0, "oldPath")
  JERRYXX_CHECK_ARG_STRING_OPT(1, "newPath")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, old_path)
  JERRYXX_GET_ARG_STRING_AS_CHAR(1, new_path)

  // get native vfs handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);

  // file rename
  int ret = lfs_rename(&vfs_handle->lfs, old_path, new_path);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }
  return jerry_undefined();
}

/**
 * VFSLittleFS.prototype.unlink()
 * args:
 *   path {string}
 */
JERRYXX_FUN(vfs_lfs_unlink_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_STRING_OPT(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)

  // get native vfs handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info)

  // file delete
  int ret = lfs_remove(&vfs_handle->lfs, path);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }
  return jerry_undefined();
}

/**
 * VFSLittleFS.prototype.mkdir()
 * args:
 *   path {string}
 */
JERRYXX_FUN(vfs_lfs_mkdir_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)

  // get native vfs handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info)

  // create a directory
  int ret = lfs_mkdir(&vfs_handle->lfs, path);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }
  return jerry_undefined();
}

/**
 * VFSLittleFS.prototype.readdir()
 * args:
 *   path {string}
 * returns {string[]} - array of filenames
 */
JERRYXX_FUN(vfs_lfs_readdir_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)

  // get native vfs handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);

  // read dir
  lfs_dir_t dir;
  struct lfs_info info;
  int ret = lfs_dir_open(&vfs_handle->lfs, &dir, path);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }
  jerry_value_t files = jerry_array(0);
  while (true) {
    ret = lfs_dir_read(&vfs_handle->lfs, &dir, &info);
    if (ret < 0) {
      return jerry_exception_value(create_system_error(ret), true);
    }
    if (ret == 0) {
      break;
    }
    // add to array except '.', '..'
    if (strcmp(info.name, ".") != 0 && strcmp(info.name, "..") != 0) {
      jerry_value_t item =
          jerry_string_sz((const char *)(info.name));
      jerryxx_array_push_string(files, item);
      jerry_value_free(item);
    }
  }
  ret = lfs_dir_close(&vfs_handle->lfs, &dir);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }
  return files;
}

/**
 * VFSLittleFS.prototype.rmdir()
 * args:
 *   path {string}
 */
JERRYXX_FUN(vfs_lfs_rmdir_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_STRING_OPT(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)

  // get native vfs handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_lfs_handle_t, vfs_handle_info);

  // remove directory
  int ret = lfs_remove(&vfs_handle->lfs, path);
  if (ret < 0) {
    return jerry_exception_value(create_system_error(ret), true);
  }
  return jerry_undefined();
}

/**
 * Initialize fs_native object and return exportsio
 */
jerry_value_t module_vfs_lfs_init() {
  /* VFSLittleFS class */
  jerry_value_t vfs_lfs_ctor = jerry_function_external(vfslfs_ctor_fn);
  jerry_value_t vfs_lfs_prototype = jerry_object();
  jerryxx_set_property(vfs_lfs_ctor, MSTR_PROTOTYPE, vfs_lfs_prototype);
  jerryxx_set_property_function(vfs_lfs_prototype, MSTR_VFS_LFS_MKFS,
                                vfs_lfs_mkfs_fn);
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
  jerryxx_set_property_function(vfs_lfs_prototype, MSTR_VFS_LFS_STAT,
                                vfs_lfs_stat_fn);
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
  jerry_value_free(vfs_lfs_prototype);

  /* vfslittlefs module exports */
  jerry_value_t exports = jerry_object();
  jerryxx_set_property(exports, MSTR_VFS_LFS_VFSLITTLEFS, vfs_lfs_ctor);
  jerry_value_free(vfs_lfs_ctor);

  return exports;
}
