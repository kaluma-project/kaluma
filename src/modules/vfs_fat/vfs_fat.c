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

#include "vfs_fat.h"

#include <stdlib.h>

#include "err.h"
#include "ff.h"
#include "gpio.h"
#include "rtc.h"
#include "spi.h"
#include "system.h"

vfs_fat_root_t vfs_fat_root;

void vfs_fat_init() {
  vfs_fat_root.file_id_count = 0;
  vfs_fat_root.fs_count = 0;
  km_list_init(&vfs_fat_root.vfs_fat_handles);
}

void vfs_fat_cleanup() {
  vfs_fat_handle_t *handle =
      (vfs_fat_handle_t *)vfs_fat_root.vfs_fat_handles.head;
  while (handle != NULL) {
    vfs_fat_handle_t *next =
        (vfs_fat_handle_t *)((km_list_node_t *)handle)->next;
    free(handle);
    handle = next;
  }
  km_list_init(&vfs_fat_root.vfs_fat_handles);
}

void vfs_fat_handle_init(vfs_fat_handle_t *handle) {
  km_list_init(&handle->file_handles);
}

void vfs_fat_handle_add(vfs_fat_handle_t *handle) {
  handle->fs_no = vfs_fat_root.fs_count++;
  km_list_append(&vfs_fat_root.vfs_fat_handles, (km_list_node_t *)handle);
}

void vfs_fat_handle_remove(vfs_fat_handle_t *handle) {
  km_list_remove(&vfs_fat_root.vfs_fat_handles, (km_list_node_t *)handle);
}

void vfs_fat_file_add(vfs_fat_handle_t *handle, vfs_fat_file_handle_t *file) {
  vfs_fat_root.file_id_count++;
  file->id = vfs_fat_root.file_id_count;
  km_list_append(&handle->file_handles, (km_list_node_t *)file);
}

void vfs_fat_file_remove(vfs_fat_handle_t *handle,
                         vfs_fat_file_handle_t *file) {
  km_list_remove(&handle->file_handles, (km_list_node_t *)file);
}

vfs_fat_file_handle_t *vfs_fat_file_get_by_id(vfs_fat_handle_t *handle,
                                              uint32_t file_id) {
  vfs_fat_file_handle_t *file =
      (vfs_fat_file_handle_t *)handle->file_handles.head;
  while (file != NULL) {
    if (file->id == file_id) {
      return file;
    }
    file = (vfs_fat_file_handle_t *)((km_list_node_t *)file)->next;
  }
  return NULL;
}

vfs_fat_handle_t *vfs_fat_get_fs_by_drv(vfs_fat_root_t *handle, BYTE pdrv) {
  vfs_fat_handle_t *vfs_fs = (vfs_fat_handle_t *)handle->vfs_fat_handles.head;
  while (vfs_fs != NULL) {
    if (vfs_fs->fs_no == pdrv) {
      return vfs_fs;
    }
    vfs_fs = (vfs_fat_handle_t *)((km_list_node_t *)vfs_fs)->next;
  }
  return NULL;
}