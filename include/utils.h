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

#ifndef __KM_UTILS_H
#define __KM_UTILS_H

#include <stdint.h>

typedef struct km_list_node_s km_list_node_t;
typedef struct km_list_s km_list_t;

struct km_list_node_s {
  km_list_node_t *prev;
  km_list_node_t *next;
};

struct km_list_s {
  km_list_node_t *head;
  km_list_node_t *tail;
};

void km_list_init(km_list_t *list);
void km_list_append(km_list_t *list, km_list_node_t *node);
void km_list_remove(km_list_t *list, km_list_node_t *node);

uint8_t km_hex1(char hex);
uint8_t km_hex2bin(unsigned char *hex);
#endif /* __KM_UTILS_H */