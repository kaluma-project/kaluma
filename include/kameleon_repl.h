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

#ifndef __KAMELEON_REPL_H
#define __KAMELEON_REPL_H

#define MAX_BUFFER_LENGTH 1024
#define MAX_COMMAND_HISTORY 10

typedef enum {
  NORMAL,
  ESCAPE
} repl_mode_t;

struct repl_state_s;

typedef void (*repl_input_handler_t)(struct repl_state_s *, char);

struct repl_state_s {
  repl_mode_t mode;
  bool echo;
  repl_input_handler_t input_handler;
  char buffer[MAX_BUFFER_LENGTH];
  unsigned int buffer_length;
  unsigned int position;
  char escape[3];
  unsigned int escape_length;
  char *history[MAX_COMMAND_HISTORY];
  unsigned int history_size;
  unsigned int history_position;
};

typedef struct repl_state_s repl_state_t;

void repl_init();
void repl_prompt();
void repl_putc(char ch);
void repl_set_input_handler(repl_input_handler_t handler);
void repl_log(const char *format, const char *str);
void repl_info(const char *format, const char *str);
void repl_error(const char *format, const char *str);

#endif /* __KAMELEON_REPL_H */