/* Copyright (c) 2017 Kameleon
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

#ifndef __KAMELEON_LIST_H
#define __KAMELEON_LIST_H

typedef struct kameleon_list_node_s kameleon_list_node_t;
typedef struct kameleon_list_s kameleon_list_t;

struct kameleon_list_node_s {
  kameleon_list_node_t *prev;
  kameleon_list_node_t *next;
};

struct kameleon_list_s {
  kameleon_list_node_t *head;
  kameleon_list_node_t *tail;
};

void kameleon_list_init(kameleon_list_t *list);
void kameleon_list_append(kameleon_list_t *list, kameleon_list_node_t *node);
void kameleon_list_remove(kameleon_list_t *list, kameleon_list_node_t *node);

#endif /* __KAMELEON_LIST_H */