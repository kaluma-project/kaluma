/* Copyright (c) 2017 Kalamu
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

#ifndef __REPL_H
#define __REPL_H

#include "jerryscript.h"
#include "jerryxx.h"

#define MAX_BUFFER_LENGTH 1024
#define MAX_COMMAND_HISTORY 10

typedef enum {
  REPL_MODE_NORMAL,
  REPL_MODE_ESCAPE
} repl_mode_t;

typedef enum {
  REPL_OUTPUT_NORMAL,
  REPL_OUTPUT_INFO,
  REPL_OUTPUT_ERROR
} repl_output_t;

typedef struct repl_state_s repl_state_t;
typedef void (*repl_handler_t)(repl_state_t *, uint8_t *, size_t);

struct repl_state_s {
  repl_mode_t mode;
  bool echo;
  repl_handler_t handler;
  char buffer[MAX_BUFFER_LENGTH + 1];
  unsigned int buffer_length;
  unsigned int position;
  unsigned int width;
  char escape[16];
  unsigned int escape_length;
  char *history[MAX_COMMAND_HISTORY];
  unsigned int history_size;
  unsigned int history_position;
  uint8_t ymodem_state; // 0=stopped, 1=transfering
};

void repl_init();
repl_state_t *get_repl_state();

void repl_set_output(repl_output_t output);
void repl_print_prompt();
#define repl_printf(format,args...) tty_printf(format, ## args)
#define repl_print_value(value) jerryxx_print_value(value)
#define repl_putc(ch) tty_putc(ch)
void repl_pretty_print(uint8_t indent, uint8_t depth, jerry_value_t value);
void repl_println();

#endif /* __REPL_H */