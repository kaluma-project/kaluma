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

#ifndef __VFS_H
#define __VFS_H

#include "jerryscript.h"
#include "utils.h"

// File flags
enum vfs_flags {
  VFS_FLAG_READ = 1,
  VFS_FLAG_WRITE = 2,
  VFS_FLAG_CREATE = 4,
  VFS_FLAG_APPEND = 8,
  VFS_FLAG_EXCL = 16,
  VFS_FLAG_TRUNC = 32,
};

typedef struct vfs_s vfs_t;
typedef struct vfs_mount_table_s vfs_mount_table_t;

struct vfs_s {
  void *handle;  // e.g. lfs_t
  // vfs_mkfs_cb(void* vfs);
  // vfs_mount_cb;
  // vfs_unmount_cb;
  // vfs_mkdir_cb(void *vfs, char* path);
  // vfs_rmdir_cb;
  // vfs_readdir_cb;
  // vfs_open_cb(path, flag, mode) -> id;
  // vfs_write_cb;
  // vfs_read_cb;
  // vfs_close_cb;
  // vfs_stat_cb;
  // vfs_unlink_cb;
  // vfs_rename_cb;
}

struct vfs_mount_table_s {
  vfs_mount_table_t *next;
  char *path;
  vfs_t *vfs;
};

// mount table
vfs_mount_table_t *vfs_mount_table_root;

// current working directory
char *cwd;

int vfs_mkfs(jerry_value_t blkdev);  // set blkdev.vfs_id = 'lfs'
int vfs_mount(jerry_value_t blkdev, char *path);
int vfs_unmount(char *path);
int vfs_chdir(char *path);

// vfs_mkdir
// vfs_rmdir
// vfs_open
// ...

#endif /* __VFS_H */
