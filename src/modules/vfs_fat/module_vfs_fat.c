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

#include "diskio.h"
#include "err.h"
#include "ff.h"
#include "io.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "magic_strings.h"
#include "tty.h"
#include "vfs_fat.h"
#include "vfs_fat_magic_strings.h"

extern vfs_fat_root_t vfs_fat_root;

static void vfs_handle_freecb(void *handle) {
  vfs_fat_handle_t *vfs_handle = (vfs_fat_handle_t *)handle;
  jerry_release_value(vfs_handle->blkdev_js);
  free(vfs_handle->fat_fs);
  free(handle);
}

static const jerry_object_native_info_t vfs_handle_info = {
    .free_cb = vfs_handle_freecb};

static int blkdev_ioctl(jerry_value_t blkdev_js, int op, int arg) {
  // km_tty_printf("blkdev_ioctl(%d, %d)\r\n", op, arg);
  jerry_value_t ioctl_js = jerryxx_get_property(blkdev_js, "ioctl");
  jerry_value_t op_js = jerry_create_number(op);
  jerry_value_t arg_js = jerry_create_number(arg);
  jerry_value_t args[2] = {op_js, arg_js};
  jerry_value_t ret = jerry_call_function(ioctl_js, blkdev_js, args, 2);
  int ret_value = 0;
  if (jerry_value_is_number(ret)) {
    ret_value = (int)jerry_get_number_value(ret);
  }
  jerry_release_value(ret);
  jerry_release_value(arg_js);
  jerry_release_value(op_js);
  jerry_release_value(ioctl_js);
  // km_tty_printf("blkdev_ioctl() ->  %d\r\n", ret_value);
  return ret_value;
}

DSTATUS disk_status(BYTE pdrv) /* [IN] Physical drive number */
{
  VFS_GET_FS_HANDLE(&vfs_fat_root, vfs_handle, pdrv);

  return vfs_handle->status;
}

DSTATUS disk_initialize(BYTE pdrv) /* [IN] Physical drive number */
{
  // get native vfs handle
  VFS_GET_FS_HANDLE(&vfs_fat_root, vfs_handle, pdrv);
  vfs_handle->status &= ~STA_NOINIT;
  return vfs_handle->status;
}

DRESULT disk_read(BYTE pdrv,    /* [IN] Physical drive number */
                  BYTE *buff,   /* [OUT] Pointer to the read data buffer */
                  DWORD sector, /* [IN] Start sector number */
                  UINT count    /* [IN] Number of sectros to read */
) {
  if (count == 0) {  // Support drive 0 only
    return RES_PARERR;
  }
  // get native vfs handle
  VFS_GET_FS_HANDLE(&vfs_fat_root, vfs_handle, pdrv);
  int block_size = blkdev_ioctl(vfs_handle->blkdev_js, 5, 0);
  jerry_value_t arraybuffer = jerry_create_arraybuffer_external(
      count * block_size, (uint8_t *)buff, NULL);
  jerry_value_t buffer_js = jerry_create_typedarray_for_arraybuffer(
      JERRY_TYPEDARRAY_UINT8, arraybuffer);
  jerry_value_t read_js = jerryxx_get_property(vfs_handle->blkdev_js, "read");
  jerry_value_t block_js = jerry_create_number(sector);
  jerry_value_t offset_js = jerry_create_number(0);
  jerry_value_t args[3] = {block_js, buffer_js, offset_js};
  jerry_value_t ret =
      jerry_call_function(read_js, vfs_handle->blkdev_js, args, 3);
  jerry_release_value(ret);
  jerry_release_value(offset_js);
  jerry_release_value(block_js);
  jerry_release_value(read_js);
  jerry_release_value(buffer_js);
  jerry_release_value(arraybuffer);
  return RES_OK;
}

DRESULT disk_write(
    BYTE pdrv,        /* [IN] Physical drive number */
    const BYTE *buff, /* [IN] Pointer to the data to be written */
    DWORD sector,     /* [IN] Sector number to write from */
    UINT count        /* [IN] Number of sectors to write */
) {
  if (count == 0) {  // Support drive 0 only
    return RES_PARERR;
  }
  //  get native vfs handle
  VFS_GET_FS_HANDLE(&vfs_fat_root, vfs_handle, pdrv);
  int block_size = blkdev_ioctl(vfs_handle->blkdev_js, 5, 0);
  jerry_value_t arraybuffer = jerry_create_arraybuffer_external(
      count * block_size, (uint8_t *)buff, NULL);
  jerry_value_t buffer_js = jerry_create_typedarray_for_arraybuffer(
      JERRY_TYPEDARRAY_UINT8, arraybuffer);
  jerry_value_t write_js = jerryxx_get_property(vfs_handle->blkdev_js, "write");
  jerry_value_t block_js = jerry_create_number(sector);
  jerry_value_t offset_js = jerry_create_number(0);
  jerry_value_t args[3] = {block_js, buffer_js, offset_js};
  jerry_value_t ret =
      jerry_call_function(write_js, vfs_handle->blkdev_js, args, 3);
  jerry_release_value(ret);
  jerry_release_value(offset_js);
  jerry_release_value(block_js);
  jerry_release_value(write_js);
  jerry_release_value(buffer_js);
  jerry_release_value(arraybuffer);
  return RES_OK;
}

DRESULT disk_ioctl(BYTE pdrv, /* [IN] Drive number */
                   BYTE cmd,  /* [IN] Control command code */
                   void *buff /* [I/O] Parameter and data buffer */
) {
  //  get native vfs handle
  VFS_GET_FS_HANDLE(&vfs_fat_root, vfs_handle, pdrv);
  if (vfs_handle->status & STA_NOINIT) {
    return RES_NOTRDY;
  }
  DRESULT ret = RES_ERROR;
  switch (cmd) {
    int res;
    case CTRL_SYNC:
      blkdev_ioctl(vfs_handle->blkdev_js, 3, 0);
      ret = RES_OK;
      break;
    case GET_SECTOR_COUNT:
      res = blkdev_ioctl(vfs_handle->blkdev_js, 4, 0);
      *(DWORD *)buff = (DWORD)res;
      ret = RES_OK;
      break;
    case GET_SECTOR_SIZE:
      res = blkdev_ioctl(vfs_handle->blkdev_js, 5, 0);
      *(DWORD *)buff = (DWORD)res;
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
  // uint64_t rtc = km_rtc_get_time();
  timestamp.value = 0;
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
  jerry_acquire_value(vfs_handle->blkdev_js);
  vfs_handle->fat_fs = (FATFS *)malloc(sizeof(FATFS));
  vfs_handle->status = STA_NOINIT;
  // assign native handle in js object
  jerry_set_object_native_pointer(this_val, vfs_handle, &vfs_handle_info);
  return jerry_create_undefined();
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
  char logic_drv[7];
  // sprintf(logic_drv, "%d://", vfs_handle->fs_no);
  sprintf(logic_drv, "/");
  FRESULT ret = f_mkfs(logic_drv, FM_ANY, 0, buff, buff_size);
  free(buff);
  if (ret != FR_OK) {
    return jerry_create_error_from_value(create_system_error(-5), true);
  }
  return jerry_create_undefined();
}

/**
 * VFSFAT.prototype.mount()
 */
JERRYXX_FUN(vfs_fat_mount_fn) {
  // get native vfs handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);

  // initialize block device
  blkdev_ioctl(vfs_handle->blkdev_js, 1, 0);

  // mount vfs
  char logic_drv[7];
  // sprintf(logic_drv, "%d://", vfs_handle->fs_no);
  sprintf(logic_drv, "/");
  FRESULT ret = f_mount(vfs_handle->fat_fs, logic_drv, 1);
  if (ret != FR_OK) {
    return jerry_create_error_from_value(create_system_error(-1), true);
  }
  return jerry_create_undefined();
}

/**
 * VFSFAT.prototype.unmount()
 */
JERRYXX_FUN(vfs_fat_unmount_fn) {
  // get native vfs handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);

  // unmount vfs
  char logic_drv[7];
  // sprintf(logic_drv, "%d://", vfs_handle->fs_no);
  sprintf(logic_drv, "/");
  FRESULT ret = f_mount(NULL, logic_drv, 0);
  if (ret != FR_OK) {
    return jerry_create_error_from_value(create_system_error(-1), true);
  }

  // shutdown block device
  blkdev_ioctl(vfs_handle->blkdev_js, 2, 0);
  return jerry_create_undefined();
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
  if (flags & VFS_FLAG_EXCL) fat_flags |= FA_OPEN_EXISTING;
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
  FRESULT ret = f_open(file->fat_fp, path, fat_flags);
  if (ret != FR_OK) {
    return jerry_create_error_from_value(create_system_error(-5), true);
  }

  // add file handle and return the id
  vfs_fat_file_add(vfs_handle, file);
  uint32_t id = file->id;
  return jerry_create_number(id);
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
      jerry_get_typedarray_buffer(buffer, &buf_offset, &buf_length);
  uint8_t *buffer_p = jerry_get_arraybuffer_pointer(arrbuf);
  jerry_release_value(arrbuf);
  UINT offset = (UINT)JERRYXX_GET_ARG_NUMBER_OPT(2, 0);
  UINT length = (UINT)JERRYXX_GET_ARG_NUMBER_OPT(3, buf_length);
  FSIZE_t position = (FSIZE_t)JERRYXX_GET_ARG_NUMBER_OPT(4, -1);

  // get native file handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);
  VFS_FAT_GET_FILE_HANDLE(vfs_handle, file, id)

  // set position
  if (position > -1) {
    FRESULT pos = f_lseek(file->fat_fp, position);
    if (pos != FR_OK) {
      return jerry_create_error_from_value(create_system_error(-5), true);
    }
  }

  // file write
  UINT out_length = 0;
  FRESULT ret =
      f_write(file->fat_fp, (void *)(buffer_p + offset), length, &out_length);
  if (ret != FR_OK) {
    return jerry_create_error_from_value(create_system_error(-5), true);
  }

  // return number of bytes written
  return jerry_create_number(out_length);
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
      jerry_get_typedarray_buffer(buffer, &buf_offset, &buf_length);
  uint8_t *buffer_p = jerry_get_arraybuffer_pointer(arrbuf);
  jerry_release_value(arrbuf);
  UINT offset = (UINT)JERRYXX_GET_ARG_NUMBER_OPT(2, 0);
  UINT length = (UINT)JERRYXX_GET_ARG_NUMBER_OPT(3, buf_length);
  FSIZE_t position = (FSIZE_t)JERRYXX_GET_ARG_NUMBER_OPT(4, -1);

  // get native file handle
  JERRYXX_GET_NATIVE_HANDLE(vfs_handle, vfs_fat_handle_t, vfs_handle_info);
  VFS_FAT_GET_FILE_HANDLE(vfs_handle, file, id)

  // set position
  if (position > -1) {
    FRESULT pos = f_lseek(file->fat_fp, position);
    if (pos != FR_OK) {
      return jerry_create_error_from_value(create_system_error(-5), true);
    }
  }

  // file read
  UINT out_length = 0;
  FRESULT ret =
      f_read(file->fat_fp, (void *)(buffer_p + offset), length, &out_length);
  if (ret != FR_OK) {
    return jerry_create_error_from_value(create_system_error(-5), true);
  }

  // return number of bytes read
  return jerry_create_number(out_length);
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
  if (ret != FR_OK) {
    return jerry_create_error_from_value(create_system_error(-5), true);
  }
  free(file->fat_fp);
  // remote file handle
  vfs_fat_file_remove(vfs_handle, file);
  free(file);

  // return
  return jerry_create_undefined();
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

  // file stat
  FILINFO *info = (FILINFO *)malloc(sizeof(FILINFO));
  int ret = f_stat(path, info);
  if (ret != FR_OK) {
    free(info);
    return jerry_create_error_from_value(create_system_error(-5), true);
  }

  // return stat object {type, size}
  jerry_value_t obj = jerry_create_object();
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
  int ret = f_rename(old_path, new_path);
  if (ret != FR_OK) {
    return jerry_create_error_from_value(create_system_error(-5), true);
  }
  return jerry_create_undefined();
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
  int ret = f_unlink(path);
  if (ret != FR_OK) {
    return jerry_create_error_from_value(create_system_error(-5), true);
  }
  return jerry_create_undefined();
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
  int ret = f_mkdir(path);
  if (ret != FR_OK) {
    return jerry_create_error_from_value(create_system_error(-5), true);
  }
  return jerry_create_undefined();
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
  DIR dir;
  FILINFO *info = (FILINFO *)malloc(sizeof(FILINFO));
  FRESULT ret = f_opendir(&dir, path);
  if (ret != FR_OK) {
    return jerry_create_error_from_value(create_system_error(-5), true);
  }
  jerry_value_t files = jerry_create_array(0);
  while (true) {
    ret = f_readdir(&dir, info);
    if (ret != FR_OK) {
      return jerry_create_error_from_value(create_system_error(-5), true);
    }
    if (info->fname[0] == 0) {
      break;
    }
    // add to array except '.', '..'
    if (strcmp(info->fname, ".") != 0 && strcmp(info->fname, "..") != 0) {
      jerry_value_t item =
          jerry_create_string((const jerry_char_t *)(info->fname));
      jerryxx_array_push_string(files, item);
      jerry_release_value(item);
    }
  }
  free(info);
  ret = f_closedir(&dir);
  if (ret != FR_OK) {
    return jerry_create_error_from_value(create_system_error(-5), true);
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
  int ret = f_unlink(path);
  if (ret != FR_OK) {
    return jerry_create_error_from_value(create_system_error(-5), true);
  }
  return jerry_create_undefined();
}

/**
 * Initialize fs_native object and return exportsio
 */
jerry_value_t module_vfs_fat_init() {
  vfs_fat_init();
  /* VFSFat class */
  jerry_value_t vfs_fat_ctor = jerry_create_external_function(vfsfat_ctor_fn);
  jerry_value_t vfs_fat_prototype = jerry_create_object();
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
  jerry_release_value(vfs_fat_prototype);

  /* VFSFatFS module exports */
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property(exports, MSTR_VFS_FAT_VFSFAT, vfs_fat_ctor);
  jerry_release_value(vfs_fat_ctor);

  return exports;
}
