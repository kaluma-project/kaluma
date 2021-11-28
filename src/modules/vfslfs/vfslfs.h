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

typedef struct vfslfs_handle_s vfslfs_handle_t;
typedef struct vfslfs_file_handle_s vfslfs_file_handle_t;

/**
 * VFSLittleFS native handle
 */
struct vfslfs_handle_s {
  lfs_t lfs;
  struct lfs_config config;
  km_list_t file_handles;
  jerry_value_t blockdev_js;
};

struct vfslfs_file_handle_s {
  km_list_node_t base;
  uint32_t id;
  lfs_file_t lfs_file;
};

void vfslfs_file_add(vfslfs_handle_t *, vfslfs_file_handle_t *);
void vfslfs_file_remove(vfslfs_handle_t *, vfslfs_file_handle_t *);
vfslfs_file_handle_t *vfslfs_file_get(vfslfs_handle_t *, uint32_t);

#endif /* __VFSLFS_H */