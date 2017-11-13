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

typedef struct list_node_s list_node_t;
typedef struct list_s list_t;

struct list_node_s {
  list_node_t *prev; 
  list_node_t *next;
};

struct list_s {
  list_node_t *head;
  list_node_t *tail;
};

struct text_node_s {
  list_node_t base;
  char *text;  
};

typedef struct text_node_s text_node_t;

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

int main(void) {
  list_t list;

  printf("start...\n");

  text_node_t *node1 = (text_node_t *) malloc(sizeof(text_node_t));
  node1->text = "node1";
  text_node_t *node2 = (text_node_t *) malloc(sizeof(text_node_t));
  node2->text = "node2";
  text_node_t *node3 = (text_node_t *) malloc(sizeof(text_node_t));
  node3->text = "node3";
  text_node_t *node4 = (text_node_t *) malloc(sizeof(text_node_t));
  node4->text = "node4";
  /*
  text_node_t node2 = {
    .prev = NULL,
    .next = NULL,
    .text = "node2"
  };
  text_node_t node3 = {
    .prev = NULL,
    .next = NULL,
    .text = "node3"
  };
  text_node_t node4 = {
    .prev = NULL,
    .next = NULL,
    .text = "node4"
  };
  */

  printf("nodes are created...\n");
  
  list_init(&list);
  printf("1...\n");
  
  list_append(&list, (list_node_t *) node1);
  printf("2...\n");  
  printf("%s\n", node1->text);

  list_append(&list, (list_node_t *) node2);
  printf("3...\n");
  printf("%s\n", node2->text);

  list_append(&list, (list_node_t *) node3);
  printf("4...\n");
  printf("%s\n", node3->text);

  list_append(&list, (list_node_t *) node4);
  printf("5...\n");
  printf("%s\n", node4->text);

  printf("nodes are appended...\n");  

  list_node_t *p = list.head;
  while (p != NULL) {
    text_node_t *tn = (text_node_t *) p;
    printf("%s\n", tn->text);
    p = p->next;
  }
}