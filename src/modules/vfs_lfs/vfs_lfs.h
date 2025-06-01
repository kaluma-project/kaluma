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

#ifndef __VFSLFS_H
#define __VFSLFS_H

#include "jerryscript.h"
#include "lfs.h"
#include "utils.h"

#define VFS_LFS_GET_FILE_HANDLE(vfs_handle, file, id)                    \
  vfs_lfs_file_handle_t *file = vfs_lfs_file_get_by_id(vfs_handle, id);  \
  if (file == NULL) {                                                    \
    return jerry_exception_value(create_system_error(-9), true); \
  }

typedef struct vfs_lfs_root_s vfs_lfs_root_t;
typedef struct vfs_lfs_handle_s vfs_lfs_handle_t;
typedef struct vfs_lfs_file_handle_s vfs_lfs_file_handle_t;

// File open flags
enum vfs_lfs_open_flags {
  VFS_FLAG_READ = 1,
  VFS_FLAG_WRITE = 2,
  VFS_FLAG_CREATE = 4,
  VFS_FLAG_APPEND = 8,
  VFS_FLAG_EXCL = 16,
  VFS_FLAG_TRUNC = 32,
};

struct vfs_lfs_root_s {
  uint32_t file_id_count;
  km_list_t vfs_lfs_handles;
};

struct vfs_lfs_handle_s {
  km_list_node_t base;
  lfs_t lfs;
  struct lfs_config config;
  km_list_t file_handles;
  jerry_value_t blkdev_js;
};

struct vfs_lfs_file_handle_s {
  km_list_node_t base;
  uint32_t id;
  lfs_file_t lfs_file;
};

void vfs_lfs_init();
void vfs_lfs_cleanup();
void vfs_lfs_handle_init(vfs_lfs_handle_t *);
void vfs_lfs_handle_add(vfs_lfs_handle_t *);
void vfs_lfs_handle_remove(vfs_lfs_handle_t *);
void vfs_lfs_file_add(vfs_lfs_handle_t *, vfs_lfs_file_handle_t *);
void vfs_lfs_file_remove(vfs_lfs_handle_t *, vfs_lfs_file_handle_t *);
vfs_lfs_file_handle_t *vfs_lfs_file_get_by_id(vfs_lfs_handle_t *, uint32_t);

#endif /* __VFSLFS_H */