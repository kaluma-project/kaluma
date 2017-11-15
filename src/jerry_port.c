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
#include <string.h>

#include "jerryscript-ext/handler.h"
#include "jerryscript-port.h"
#include "jerryscript.h"

#include "tty.h"
#include "repl.h"

/**
 * Aborts the program.
 */
void jerry_port_fatal(jerry_fatal_code_t code) {
  exit(1);
} /* jerry_port_fatal */

/**
 * Provide log message implementation for the engine.
 */
void jerry_port_log(jerry_log_level_t level, /**< log level */
                    const char *format,      /**< format string */
                    ...) {                   /**< parameters */
  /* Drain log messages since IoT.js has not support log levels yet. */
  // repl_warning(format, format);
} /* jerry_port_log */

/**
 * Dummy function to get the time zone.
 *
 * @return true
 */
bool jerry_port_get_time_zone(jerry_time_zone_t *tz_p) {
  /* We live in UTC. */
  tz_p->offset = 0;
  tz_p->daylight_saving_time = 0;

  return true;
} /* jerry_port_get_time_zone */

/**
 * Dummy function to get the current time.
 *
 * @return 0
 */
double jerry_port_get_current_time(void) {
  return 0;
} /* jerry_port_get_current_time */

/**
 * Provide the implementation of jerryx_port_handler_print_char.
 * Uses 'printf' to print a single character to standard output.
 */
void jerryx_port_handler_print_char(char c) { /**< the character to print */
  tty_putc(c);
} /* jerryx_port_handler_print_char */