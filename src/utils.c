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

#include "utils.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void km_list_init(km_list_t *list) {
  list->head = NULL;
  list->tail = NULL;
}

void km_list_append(km_list_t *list, km_list_node_t *node) {
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

void km_list_remove(km_list_t *list, km_list_node_t *node) {
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

uint8_t km_hex1(char hex) {
  if (hex >= 'a') {
    return (hex - 'a' + 10);
  } else if (hex >= 'A') {
    return (hex - 'A' + 10);
  } else {
    return (hex - '0');
  }
}

uint8_t km_hex2bin(unsigned char *hex) {
  uint8_t hh = km_hex1(hex[0]);
  uint8_t hl = km_hex1(hex[1]);
  return hh << 4 | hl;
}

#define LEAPS_THRU_END_OF(y) ((y) / 4 - (y) / 100 + (y) / 400)

static inline bool is_leap_year(unsigned int year) {
  return (!(year % 4) && (year % 100)) || !(year % 400);
}

/*
 * The number of days in the month.
 */
static int rtc_month_days(unsigned int month, unsigned int year) {
  const unsigned char rtc_days_in_month[] = {31, 28, 31, 30, 31, 30,
                                             31, 31, 30, 31, 30, 31};
  return rtc_days_in_month[month] + (is_leap_year(year) && month == 1);
}

void km_rtc_to_time(uint64_t time_ms, struct km_time *tm) {
  uint8_t month;
  uint16_t year;
  uint64_t secs, days;
  uint64_t time_s = time_ms / 1000;
  secs = time_s % 86400;
  days = time_s / 86400;

  /* day of the week, 1970-01-01 was a Thursday */
  tm->week_day = (days + 4) % 7;

  year = 1970 + days / 365;
  days -= (year - 1970) * 365 + LEAPS_THRU_END_OF(year - 1) -
          LEAPS_THRU_END_OF(1970 - 1);
  while (days < 0) {
    year -= 1;
    days += 365 + is_leap_year(year);
  }
  tm->year = year;

  for (month = 0; month < 11; month++) {
    int newdays;

    newdays = days - rtc_month_days(month, year);
    if (newdays < 0) break;
    days = newdays;
  }
  tm->mon = month + 1;
  tm->day = days + 1;

  tm->hour = secs / 3600;
  secs -= tm->hour * 3600;
  tm->min = secs / 60;
  tm->sec = secs - tm->min * 60;
}