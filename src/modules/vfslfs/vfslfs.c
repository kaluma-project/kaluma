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

#include "vfslfs.h"

#include <stdlib.h>

#include "lfs.h"
#include "utils.h"

void vfslfs_file_add(vfslfs_handle_t *handle, vfslfs_file_handle_t *file) {
  km_list_append(&handle->file_handles, (km_list_node_t *)file);
}

void vfslfs_file_remove(vfslfs_handle_t *handle, vfslfs_file_handle_t *file) {
  km_list_remove(&handle->file_handles, (km_list_node_t *)file);
}

vfslfs_file_handle_t *vfslfs_file_get(vfslfs_handle_t *handle,
                                      uint32_t file_id) {
  vfslfs_file_handle_t *file =
      (vfslfs_file_handle_t *)handle->file_handles.head;
  while (file != NULL) {
    if (file->lfs_file.id == file_id) {
      return file;
    }
    file = (vfslfs_file_handle_t *)((km_list_node_t *)file)->next;
  }
  return NULL;
}
