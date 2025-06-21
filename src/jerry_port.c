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
#include <time.h>

// #include "jerryscript-ext/handler.h"
#include "jerryscript-port.h"
#include "jerryscript.h"
#include "rtc.h"
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
void jerry_port_log (const char *message_p) {
  km_tty_printf("%s\r", message_p);
} /* jerry_port_log */

double jerry_port_get_local_time_zone_adjustment(double unix_ms, bool is_utc) {
  return 0;
}

/**
 * function to get the current time.
 */
double jerry_port_get_current_time(void) { return (double)km_rtc_get_time(); }

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
  return jerry_undefined();
} /* jerry_port_get_native_module */

/**
 * Default implementation of jerry_port_init. Do nothing.
 */
void JERRY_ATTR_WEAK
jerry_port_init (void)
{
} /* jerry_port_init */

jerry_char_t *JERRY_ATTR_WEAK
jerry_port_path_normalize (const jerry_char_t *path_p, jerry_size_t path_size)
{
  jerry_char_t *buffer_p = (jerry_char_t *) malloc (path_size + 1);

  if (buffer_p == NULL)
  {
    return NULL;
  }

  /* Also copy terminating zero byte. */
  memcpy (buffer_p, path_p, path_size + 1);

  return buffer_p;
} /* jerry_port_path_normalize */

void JERRY_ATTR_WEAK
jerry_port_path_free (jerry_char_t *path_p)
{
  free (path_p);
} /* jerry_port_path_free */

jerry_char_t *JERRY_ATTR_WEAK
jerry_port_source_read (const char *file_name_p, jerry_size_t *out_size_p)
{
  return NULL;
}

double
jerry_port_current_time (void)
{
  km_rtc_init();
  return (double)km_rtc_get_time();
} /* jerry_port_current_time */

int32_t
jerry_port_local_tza (double unix_ms)
{
  (void) unix_ms;

  /* We live in UTC. */
  return 0;
} /* jerry_port_local_tza */

void JERRY_ATTR_WEAK
jerry_port_source_free (uint8_t *buffer_p)
{
  free (buffer_p);
} /* jerry_port_source_free */

jerry_size_t JERRY_ATTR_WEAK
jerry_port_path_base (const jerry_char_t *path_p)
{
  const jerry_char_t *basename_p = (jerry_char_t *) strrchr ((char *) path_p, '/') + 1;

  return (jerry_size_t) (basename_p - path_p);
} /* jerry_port_path_base */