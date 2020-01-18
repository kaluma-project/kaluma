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

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

void list_init(list_t *list) {
  list->head = NULL;
  list->tail = NULL;
}

void list_append(list_t *list, list_node_t *node) {
  if (list->tail == NULL && list->head == NULL) {
    list->head = node;
    list->tail = node;
    node->next = NULL;
    node->prev = NULL;
  } else {
    list->tail->next = node;
    node->prev = list->tail;
    node->next = NULL;
    list->tail = node;
  }
}

void list_remove(list_t *list, list_node_t *node) {
  if (list->head == node) {
    list->head = node->next;
  }
  if (list->tail == node) {
    list->tail = node->prev;
  }
  if (node->prev != NULL) {
    node->prev->next = node->next;
  }
  if (node->next != NULL) {
    node->next->prev = node->prev;
  }
}

uint8_t hex1(char hex) {
  if (hex >= 'a') {
    return (hex - 'a' + 10);
  } else if (hex >= 'A') {
    return (hex - 'A' + 10);
  } else {
    return (hex - '0');
  }
}

uint8_t hex2bin(unsigned char *hex) {
  uint8_t hh = hex1(hex[0]);
  uint8_t hl = hex1(hex[1]);
  return hh << 4 | hl;
}

/*
int main(void) {
  list_t list;

  text_node_t *node1 = (text_node_t *) malloc(sizeof(text_node_t));
  node1->text = "node1";
  text_node_t *node2 = (text_node_t *) malloc(sizeof(text_node_t));
  node2->text = "node2";
  text_node_t *node3 = (text_node_t *) malloc(sizeof(text_node_t));
  node3->text = "node3";
  text_node_t *node4 = (text_node_t *) malloc(sizeof(text_node_t));
  node4->text = "node4";

  list_init(&list);
  list_append(&list, (list_node_t *) node1);
  list_append(&list, (list_node_t *) node2);
  list_append(&list, (list_node_t *) node3);
  list_append(&list, (list_node_t *) node4);

  list_remove(&list, (list_node_t *) node1);
  list_remove(&list, (list_node_t *) node2);
  list_remove(&list, (list_node_t *) node3);
  // list_remove(&list, (list_node_t *) node4);

  list_node_t *p = list.head;
  while (p != NULL) {
    text_node_t *tn = (text_node_t *) p;
    printf("%s\n", tn->text);
    p = p->next;
  }
  text_node_t *h = (text_node_t *) list.head;
  text_node_t *t = (text_node_t *) list.tail;
  if (h != NULL) {
    printf("head=%s\n", h->text);
  } else {
    printf("head=NULL\n");
  }
  if (t != NULL) {
    printf("tail=%s\n", t->text);
  } else {
    printf("tail=NULL\n");
  }
}
*/