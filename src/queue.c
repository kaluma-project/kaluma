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

#include <stdlib.h>
#include <stdbool.h>
#include "queue.h"

/* Initialize a queue */
int queue_init(queue_t *queue) {
  queue->head = NULL;
  queue->tail = NULL;
}

/* Test whether queue is empty or not */
bool queue_is_empty(queue_t *queue) {
  return (queue->head == NULL && queue->tail == NULL);
}

/* Push an item to the queue (at the tail) */
int queue_push(queue_t *queue, void *data) {
  queue_entry_t *entry = malloc(sizeof(queue_entry_t));
  entry->data = data;
  entry->next = NULL;
  if (queue_is_empty(queue)) {
    queue->tail = entry;
    queue->head = entry;
  } else {
    queue->tail->next = entry;
    queue->tail = entry;
  }
}

/* Pop an item from the queue (at the head) */
void *queue_pop(queue_t *queue) {
  if (queue_is_empty(queue)) {
    return NULL; // can't pop from empty queue
  } else {
    queue_entry_t *entry = queue->head;
    void *data = entry->data;
    queue->head = queue->head->next;
    free(entry);
    return data;
  }
}
