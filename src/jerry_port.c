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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jerryscript-ext/handler.h"
#include "jerryscript-port.h"
#include "jerryscript.h"
#include "tty.h"
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
  char buf[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, 256, format, args);
  km_tty_printf("%s\r", buf);
  va_end(args);
} /* jerry_port_log */

double jerry_port_get_local_time_zone_adjustment(double unix_ms, bool is_utc) {
  return 0;
}

/**
 * Dummy function to get the current time.
 *
 * @return 0
 */
double jerry_port_get_current_time(void) {
  return 0;
} /* jerry_port_get_current_time */

/**
 * Opens file with the given path and reads its source.
 * @return the source of the file
 */
uint8_t *jerry_port_read_source(const char *file_name_p, /**< file name */
                                size_t *out_size_p) /**< [out] read bytes */
{
  return NULL;
} /* jerry_port_read_source */

/**
 * Normalize a file path
 *
 * @return length of the path written to the output buffer
 */
size_t jerry_port_normalize_path(
    const char *in_path_p, /**< input file path */
    char *out_buf_p,       /**< output buffer */
    size_t out_buf_size,   /**< size of output buffer */
    char *base_file_p)     /**< base file path */
{
  // normalize in_path_p by expanding relative paths etc.
  // if base_file_p is not NULL, in_path_p is relative to that file
  // write to out_buf_p the normalized path
  // return length of written path
  return 0;
} /* jerry_port_normalize_path */

/**
 * Release the previously opened file's content.
 */
void jerry_port_release_source(uint8_t *buffer_p) /**< buffer to free */
{
  free(buffer_p);
} /* jerry_port_release_source */

/**
 * Provide the implementation of jerryx_port_handler_print_char.
 * Uses 'printf' to print a single character to standard output.
 */
void jerryx_port_handler_print_char(char c) { /**< the character to print */
  km_tty_putc(c);
} /* jerryx_port_handler_print_char */

/**
 * Get the module object of a native module.
 *
 * @return undefined
 */
jerry_value_t jerry_port_get_native_module(
    jerry_value_t name) /**< module specifier */
{
  (void)name;
  return jerry_create_undefined();
} /* jerry_port_get_native_module */