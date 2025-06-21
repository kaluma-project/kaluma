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

#include "module_vfs_fat.h"

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "diskio.h"
#include "err.h"
#include "io.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "magic_strings.h"
#include "rtc.h"
#include "tty.h"
#include "utils.h"
#include "vfs_fat.h"
#include "vfs_fat_magic_strings.h"

extern vfs_fat_root_t vfs_fat_root;

static void vfs_handle_freecb(void *handle, struct jerry_object_native_info_t *info_p) {
  vfs_fat_handle_t *vfs_handle = (vfs_fat_handle_t *)handle;
  jerry_value_free(vfs_handle->blkdev_js);
  vfs_fat_handle_remove(vfs_handle);
  free(vfs_handle->fat_fs);
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

static int ret_conversion(int ret) {
  int new_ret = 0;  // OK
  if (ret != 0) {
    switch (ret) {
      case FR_NO_FILE:
      case FR_NO_PATH:
      case FR_INVALID_NAME:
      case FR_INVALID_DRIVE:
        new_ret = ENOENT;
        break;
      case FR_EXIST:
        new_ret = EEXIST;
        break;
      case FR_WRITE_PROTECTED:
        new_ret = EROFS;
        break;
      case FR_LOCKED:
        new_ret = ETXTBSY;
        break;
      case FR_INVALID_PARAMETER:
        new_ret = EINVAL;
        break;
      default:
        new_ret = EIO;
        break;
    }
  }

  return new_ret;
}
DRESULT disk_read(void *drv,    /* [IN] Physical drive nmuber (0..) */
                  BYTE *buff,   /* [OUT] Pointer to the read data buffer */
                  DWORD sector, /* [IN] Start sector number */
                  UINT count    /* [IN] Number of sectros to read */
) {
  if (count == 0) {  // Support drive 0 only
    return RES_PARERR;
  }
  // get native vfs handle
  vfs_fat_handle_t *vfs_handle = (vfs_fat_handle_t *)drv;
  int block_size = blkdev_ioctl(vfs_handle->blkdev_js, 5, 0);
  jerry_value_t arraybuffer = jerry_arraybuffer_external(
    (uint8_t *)buff, count * block_size, NULL);
  jerry_value_t buffer_js = jerry_typedarray_with_buffer(
      JERRY_TYPEDARRAY_UINT8, arraybuffer);
  jerry_value_t read_js = jerryxx_get_property(vfs_handle->blkdev_js, "read");
  jerry_value_t block_js = jerry_number(sector);
  jerry_value_t offset_js = jerry_number(0);
  jerry_value_t args[3] = {block_js, buffer_js, offset_js};
  jerry_value_t ret =
      jerry_call(read_js, vfs_handle->blkdev_js, args, 3);
  jerry_value_free(ret);
  jerry_value_free(offset_js);
  jerry_value_free(block_js);
  jerry_value_free(read_js);
  jerry_value_free(buffer_js);
  jerry_value_free(arraybuffer);
  return RES_OK;
}

DRESULT disk_write(
    void *drv,        /* [IN] Physical drive nmuber (0..) */
    const BYTE *buff, /* [IN] Pointer to the data to be written */
    DWORD sector,     /* [IN] Sector number to write from */
    UINT count        /* [IN] Number of sectors to write */
) {
  if (count == 0) {  // Support drive 0 only
    return RES_PARERR;
  }
  // get native vfs handle
  vfs_fat_handle_t *vfs_handle = (vfs_fat_handle_t *)drv;
  int block_size = blkdev_ioctl(vfs_handle->blkdev_js, 5, 0);
  jerry_value_t arraybuffer = jerry_arraybuffer_external(
    (uint8_t *)buff, count * block_size, NULL);
  jerry_value_t buffer_js = jerry_typedarray_with_buffer(
      JERRY_TYPEDARRAY_UINT8, arraybuffer);
  jerry_value_t write_js = jerryxx_get_property(vfs_handle->blkdev_js, "write");
  jerry_value_t block_js = jerry_number(sector);
  jerry_value_t offset_js = jerry_number(0);
  jerry_value_t args[3] = {block_js, buffer_js, offset_js};
  jerry_value_t ret =
      jerry_call(write_js, vfs_handle->blkdev_js, args, 3);
  jerry_value_free(ret);
  jerry_value_free(offset_js);
  jerry_value_free(block_js);
  jerry_value_free(write_js);
  jerry_value_free(buffer_js);
  jerry_value_free(arraybuffer);
  return RES_OK;
}

DRESULT disk_ioctl(void *drv, /* [IN] Physical drive nmuber (0..) */
                   BYTE cmd,  /* [IN] Control command code */
                   void *buff /* [I/O] Parameter and data buffer */
) {
  // get native vfs handle
  vfs_fat_handle_t *vfs_handle = (vfs_fat_handle_t *)drv;
  if ((cmd != IOCTL_INIT) && (vfs_handle->status & STA_NOINIT)) {
    return RES_NOTRDY;
  }
  DRESULT ret = RES_ERROR;
  switch (cmd) {
    int res;
    case CTRL_SYNC:
      res = blkdev_ioctl(vfs_handle->blkdev_js, 3, 0);
      if (res == 0) {
        ret = RES_OK;
      }
      break;
    case GET_SECTOR_COUNT:
      res = blkdev_ioctl(vfs_handle->blkdev_js, 4, 0);
      *(DWORD *)buff = (DWORD)res;
      ret = RES_OK;
      break;
    case GET_SECTOR_SIZE:
      res = blkdev_ioctl(vfs_handle->blkdev_js, 5, 0);
      *(WORD *)buff = (WORD)res;
      ret = RES_OK;
      break;
    case GET_BLOCK_SIZE:
      res = 1;
      *(DWORD *)buff = (DWORD)res;
      ret = RES_OK;
      break;
    case CTRL_TRIM:
      ret = RES_OK;
      break;
    case IOCTL_INIT:
      res = blkdev_ioctl(vfs_handle->blkdev_js, 1, 0);
      if (res < 0) {
        break;
      }
      vfs_handle->status &= ~STA_NOINIT;
      *(DSTATUS *)buff = (DSTATUS)vfs_handle->status;
      ret = RES_OK;
      break;
    case IOCTL_STATUS:
      *(DSTATUS *)buff = (DSTATUS)vfs_handle->status;
      ret = RES_OK;
      break;
  }
  return ret;
}

DWORD get_fattime(void) {
  union {
    struct {
      DWORD second : 5;
      DWORD minute : 6;
      DWORD hour : 5;
      DWORD day_in_month : 5;
      DWORD month : 4;
      DWORD year : 7;
    };
    DWORD value;
  } timestamp;
  time_t t = (time_t)(km_rtc_get_time() / 1000);
  struct tm *tm = gmtime(&t);
  timestamp.second = (tm->tm_sec & 0x1F);
  timestamp.minute = (tm->tm_min & 0x3F);
  timestamp.hour = (tm->tm_hour & 0x1F);
  timestamp.day_in_month = (tm->tm_mday & 0x1F);
  timestamp.month = ((tm->tm_mon + 1) & 0x0F);
  timestamp.year = ((tm->tm_year + 1900 - 1980) & 0x7F);
  return timestamp.value;
}

/**
 * VFSFAT constructor
 * args:
 *   blockdev {object}
 */
JERRYXX_FUN(vfsfat_ctor_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_OBJECT(0, "blkdev")
  jerry_value_t blkdev = JERRYXX_GET_ARG(0);

  // initialize vfs native handle
  vfs_fat_handle_t *vfs_handle =
      (vfs_fat_handle_t *)malloc(sizeof(vfs_fat_handle_t));
  vfs_fat_handle_init(vfs_handle);
  vfs_fat_handle_add(vfs_handle);
  vfs_handle->blkdev_js = blkdev;
  // jerry_value_copy(vfs_handle->blkdev_js);
  vfs_handle->fat_fs = (FATFS *)malloc(sizeof(FATFS));
  vfs_handle->fat_fs->drv = (void *)vfs_handle;
  vfs_handle->status = STA_NOINIT;
  // assign native handle in js object
  jerry_object_set_native_ptr(JERRYXX_GET_THIS, &vfs_handle_info, vfs_handle);
  return jerry_undefined();
}

/**
 * VFSFAT.prototype.mkfs()
 */
JERRYXX_FUN(vfs_fat_mkfs_fn) {
  // get native vfs handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);

  // initialize block device
  blkdev_ioctl(vfs_handle->blkdev_js, 1, 0);
  int32_t buff_size = blkdev_ioctl(vfs_handle->blkdev_js, 5, 0);
  BYTE *buff = (BYTE *)malloc(sizeof(BYTE) * buff_size);
  // make fs (format)
  FRESULT ret = f_mkfs(vfs_handle->fat_fs, FM_ANY, 0, buff, buff_size);
  free(buff);
  int err = ret_conversion(ret);
  if (err < 0) {
    return jerry_exception_value(create_system_error(err), true);
  }
  return jerry_undefined();
}

/**
 * VFSFAT.prototype.mount()
 */
JERRYXX_FUN(vfs_fat_mount_fn) {
  // get native vfs handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);

  // initialize block device
  blkdev_ioctl(vfs_handle->blkdev_js, 1, 0);

  FRESULT ret = f_mount(vfs_handle->fat_fs);
  int err = ret_conversion(ret);
  if (err < 0) {
    return jerry_exception_value(create_system_error(err), true);
  }
  return jerry_undefined();
}

/**
 * VFSFAT.prototype.unmount()
 */
JERRYXX_FUN(vfs_fat_unmount_fn) {
  // get native vfs handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);

  FRESULT ret = f_umount(vfs_handle->fat_fs);
  int err = ret_conversion(ret);
  if (err < 0) {
    return jerry_exception_value(create_system_error(err), true);
  }

  // shutdown block device
  blkdev_ioctl(vfs_handle->blkdev_js, 2, 0);
  return jerry_undefined();
}

/**
 * VFSFAT.prototype.open()
 * args:
 *   path {string}
 *   flags {number} See enum vfs_fat_open_flags
 *   mode {number}
 * returns {number} - id
 */
JERRYXX_FUN(vfs_fat_open_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_CHECK_ARG_NUMBER(1, "flags")
  JERRYXX_CHECK_ARG_NUMBER(2, "mode")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  int flags = (int)JERRYXX_GET_ARG_NUMBER(1);
  // int mode = (int)JERRYXX_GET_ARG_NUMBER(2);

  // convert flags to fat_flags
  BYTE fat_flags = 0;
  if (flags & VFS_FLAG_EXCL) fat_flags |= FA_CREATE_NEW;
  if (flags & VFS_FLAG_TRUNC) fat_flags |= FA_CREATE_ALWAYS;
  if (flags & VFS_FLAG_READ) fat_flags |= FA_READ;
  if (flags & VFS_FLAG_WRITE) fat_flags |= FA_WRITE;
  if (flags & VFS_FLAG_APPEND) fat_flags |= FA_OPEN_APPEND;
  if (flags & VFS_FLAG_CREATE) fat_flags |= FA_CREATE_ALWAYS;

  // get native vfs handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);

  // create file handle
  vfs_fat_file_handle_t *file = malloc(sizeof(vfs_fat_file_handle_t));
  FIL *fp = (FIL *)malloc(sizeof(FIL));
  file->fat_fp = fp;

  // file open
  FRESULT ret = f_open(vfs_handle->fat_fs, file->fat_fp, path, fat_flags);
  int err = ret_conversion(ret);
  if (err < 0) {
    free(fp);
    free(file);
    return jerry_exception_value(create_system_error(err), true);
  }

  // add file handle and return the id
  vfs_fat_file_add(vfs_handle, file);
  uint32_t id = file->id;
  return jerry_number(id);
}

/**
 * VFSFAT.prototype.write()
 * args:
 *   id {number}
 *   buffer {TypedArray}
 *   offset {number}
 *   length {number}
 *   position {number}
 * returns {number} - number of bytes written
 */
JERRYXX_FUN(vfs_fat_write_fn) {
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
  UINT offset = (UINT)JERRYXX_GET_ARG_NUMBER_OPT(2, 0);
  UINT length = (UINT)JERRYXX_GET_ARG_NUMBER_OPT(3, buf_length);
  FSIZE_t position = (FSIZE_t)JERRYXX_GET_ARG_NUMBER_OPT(4, -1);

  // get native file handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);
  VFS_FAT_GET_FILE_HANDLE(vfs_handle, file, id)

  // set position
  if (position > -1) {
    FRESULT ret = f_lseek(file->fat_fp, position);
    int err = ret_conversion(ret);
    if (err < 0) {
      return jerry_exception_value(create_system_error(err), true);
    }
  }

  // file write
  UINT out_length = 0;
  FRESULT ret =
      f_write(file->fat_fp, (void *)(buffer_p + offset), length, &out_length);
  int err = ret_conversion(ret);
  if (err < 0) {
    return jerry_exception_value(create_system_error(err), true);
  }

  // return number of bytes written
  return jerry_number(out_length);
}

/**
 * VFSFAT.prototype.read()
 * args:
 *   id {number}
 *   buffer {TypedArray}
 *   offset {number}
 *   length {number}
 *   position {number}
 * returns {number} - number of bytes read
 */
JERRYXX_FUN(vfs_fat_read_fn) {
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
  UINT offset = (UINT)JERRYXX_GET_ARG_NUMBER_OPT(2, 0);
  UINT length = (UINT)JERRYXX_GET_ARG_NUMBER_OPT(3, buf_length);
  FSIZE_t position = (FSIZE_t)JERRYXX_GET_ARG_NUMBER_OPT(4, -1);

  // get native file handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);
  VFS_FAT_GET_FILE_HANDLE(vfs_handle, file, id)

  // set position
  if (position > -1) {
    FRESULT ret = f_lseek(file->fat_fp, position);
    int err = ret_conversion(ret);
    if (err < 0) {
      return jerry_exception_value(create_system_error(err), true);
    }
  }

  // file read
  UINT out_length = 0;
  FRESULT ret =
      f_read(file->fat_fp, (void *)(buffer_p + offset), length, &out_length);
  int err = ret_conversion(ret);
  if (err < 0) {
    return jerry_exception_value(create_system_error(err), true);
  }

  // return number of bytes read
  return jerry_number(out_length);
}

/**
 * VFSFAT.prototype.close()
 * args:
 *   id {number}
 */
JERRYXX_FUN(vfs_fat_close_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_NUMBER(0, "id")
  uint32_t id = (uint32_t)JERRYXX_GET_ARG_NUMBER(0);

  // get native file handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);
  VFS_FAT_GET_FILE_HANDLE(vfs_handle, file, id)

  // file close
  FRESULT ret = f_close(file->fat_fp);
  int err = ret_conversion(ret);
  if (err < 0) {
    return jerry_exception_value(create_system_error(err), true);
  }
  free(file->fat_fp);
  // remote file handle
  vfs_fat_file_remove(vfs_handle, file);
  free(file);

  // return
  return jerry_undefined();
}

/**
 * VFSFAT.prototype.stat()
 * args:
 *   path {string}
 * returns {fs.Stats}
 */
JERRYXX_FUN(vfs_fat_stat_fn) {  // check and get args
  // check and get args
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)
  if (strcmp(path, "/") == 0) {
    jerry_value_t obj = jerry_object();
    jerryxx_set_property_number(obj, "type", 2);
    jerryxx_set_property_number(obj, "size", 0);
    return obj;
  }
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);
  // file stat
  FILINFO *info = (FILINFO *)malloc(sizeof(FILINFO));
  FRESULT ret = f_stat(vfs_handle->fat_fs, path, info);
  int err = ret_conversion(ret);
  if (err < 0) {
    free(info);
    return jerry_exception_value(create_system_error(err), true);
  }

  // return stat object {type, size}
  jerry_value_t obj = jerry_object();
  jerryxx_set_property_number(obj, "type", info->fattrib & AM_DIR ? 2 : 1);
  jerryxx_set_property_number(obj, "size",
                              info->fattrib & AM_DIR ? 0 : info->fsize);
  free(info);
  return obj;
}

/**
 * VFSFAT.prototype.rename()
 * args:
 *   oldPath {string}
 *   newPath {string}
 */
JERRYXX_FUN(vfs_fat_rename_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_STRING_OPT(0, "oldPath")
  JERRYXX_CHECK_ARG_STRING_OPT(1, "newPath")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, old_path)
  JERRYXX_GET_ARG_STRING_AS_CHAR(1, new_path)

  // file rename
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);
  FRESULT ret = f_rename(vfs_handle->fat_fs, old_path, new_path);
  int err = ret_conversion(ret);
  if (err < 0) {
    return jerry_exception_value(create_system_error(err), true);
  }
  return jerry_undefined();
}

/**
 * VFSFAT.prototype.unlink()
 * args:
 *   path {string}
 */
JERRYXX_FUN(vfs_fat_unlink_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_STRING_OPT(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)

  // file delete
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);
  FRESULT ret = f_unlink(vfs_handle->fat_fs, path);
  int err = ret_conversion(ret);
  if (err < 0) {
    return jerry_exception_value(create_system_error(err), true);
  }
  return jerry_undefined();
}

/**
 * VFSFAT.prototype.mkdir()
 * args:
 *   path {string}
 */
JERRYXX_FUN(vfs_fat_mkdir_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)

  // create a directory
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);
  FRESULT ret = f_mkdir(vfs_handle->fat_fs, path);
  int err = ret_conversion(ret);
  if (err < 0) {
    return jerry_exception_value(create_system_error(err), true);
  }
  return jerry_undefined();
}

/**
 * VFSFAT.prototype.readdir()
 * args:
 *   path {string}
 * returns {string[]} - array of filenames
 */
JERRYXX_FUN(vfs_fat_readdir_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_STRING(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)

  // read dir
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);
  FF_DIR dir;
  FILINFO *info = (FILINFO *)malloc(sizeof(FILINFO));
  FRESULT ret = f_opendir(vfs_handle->fat_fs, &dir, path);
  int err = ret_conversion(ret);
  if (err < 0) {
    free(info);
    return jerry_exception_value(create_system_error(err), true);
  }
  jerry_value_t files = jerry_array(0);
  while (true) {
    ret = f_readdir(&dir, info);
    err = ret_conversion(ret);
    if (err < 0) {
      free(info);
      return jerry_exception_value(create_system_error(err), true);
    }
    if (info->fname[0] == 0) {
      break;
    }
    // add to array except '.', '..'
    if (strcmp(info->fname, ".") != 0 && strcmp(info->fname, "..") != 0) {
      jerry_value_t item =
          jerry_string_sz((const char *)(info->fname));
      jerryxx_array_push_string(files, item);
      jerry_value_free(item);
    }
  }
  free(info);
  ret = f_closedir(&dir);
  err = ret_conversion(ret);
  if (err < 0) {
    return jerry_exception_value(create_system_error(err), true);
  }
  return files;
}

/**
 * VFSFAT.prototype.rmdir()
 * args:
 *   path {string}
 */
JERRYXX_FUN(vfs_fat_rmdir_fn) {
  // check and get args
  JERRYXX_CHECK_ARG_STRING_OPT(0, "path")
  JERRYXX_GET_ARG_STRING_AS_CHAR(0, path)

  // file delete
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);
  FRESULT ret = f_unlink(vfs_handle->fat_fs, path);
  int err = ret_conversion(ret);
  if (err < 0) {
    return jerry_exception_value(create_system_error(err), true);
  }
  return jerry_undefined();
}

/**
 * Initialize fs_native object and return exportsio
 */
jerry_value_t module_vfs_fat_init() {
  vfs_fat_init();
  /* VFSFat class */
  jerry_value_t vfs_fat_ctor = jerry_function_external(vfsfat_ctor_fn);
  jerry_value_t vfs_fat_prototype = jerry_object();
  jerryxx_set_property(vfs_fat_ctor, MSTR_PROTOTYPE, vfs_fat_prototype);
  jerryxx_set_property_function(vfs_fat_prototype, MSTR_VFS_FAT_MKFS,
                                vfs_fat_mkfs_fn);
  jerryxx_set_property_function(vfs_fat_prototype, MSTR_VFS_FAT_MOUNT,
                                vfs_fat_mount_fn);
  jerryxx_set_property_function(vfs_fat_prototype, MSTR_VFS_FAT_UNMOUNT,
                                vfs_fat_unmount_fn);
  jerryxx_set_property_function(vfs_fat_prototype, MSTR_VFS_FAT_OPEN,
                                vfs_fat_open_fn);
  jerryxx_set_property_function(vfs_fat_prototype, MSTR_VFS_FAT_WRITE,
                                vfs_fat_write_fn);
  jerryxx_set_property_function(vfs_fat_prototype, MSTR_VFS_FAT_READ,
                                vfs_fat_read_fn);
  jerryxx_set_property_function(vfs_fat_prototype, MSTR_VFS_FAT_CLOSE,
                                vfs_fat_close_fn);
  jerryxx_set_property_function(vfs_fat_prototype, MSTR_VFS_FAT_STAT,
                                vfs_fat_stat_fn);
  jerryxx_set_property_function(vfs_fat_prototype, MSTR_VFS_FAT_RENAME,
                                vfs_fat_rename_fn);
  jerryxx_set_property_function(vfs_fat_prototype, MSTR_VFS_FAT_UNLINK,
                                vfs_fat_unlink_fn);
  jerryxx_set_property_function(vfs_fat_prototype, MSTR_VFS_FAT_MKDIR,
                                vfs_fat_mkdir_fn);
  jerryxx_set_property_function(vfs_fat_prototype, MSTR_VFS_FAT_READDIR,
                                vfs_fat_readdir_fn);
  jerryxx_set_property_function(vfs_fat_prototype, MSTR_VFS_FAT_RMDIR,
                                vfs_fat_rmdir_fn);
  jerry_value_free(vfs_fat_prototype);

  /* VFSFatFS module exports */
  jerry_value_t exports = jerry_object();
  jerryxx_set_property(exports, MSTR_VFS_FAT_VFSFAT, vfs_fat_ctor);
  jerry_value_free(vfs_fat_ctor);

  return exports;
}
