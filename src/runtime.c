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

#include <string.h>
#include "tty.h"
#include "flash.h"
#include "jerryscript.h"
#include "jerryscript-port.h"
#include "jerryscript-ext/handler.h"
#include "global.h"
#include "repl.h"
#include "runtime.h"
#include "kameleon_magic_strings.h"

// --------------------------------------------------------------------------
// PUBLIC FUNCTIONS
// --------------------------------------------------------------------------

void runtime_init(bool run_main) {
  jerry_init (JERRY_INIT_EMPTY | JERRY_INIT_MEM_STATS);
  jerry_register_magic_strings (magic_string_items, num_magic_string_items, magic_string_lengths);
  global_init();
  jerry_gc(JERRY_GC_SEVERITY_HIGH);
  if (run_main) {
    runtime_run_main();
  }
}

void runtime_deinit() {
  jerry_cleanup ();  
}


/**
 * Print error value
 */
static void
print_unhandled_exception (jerry_value_t error_value) /**< error value */
{

  error_value = jerry_get_value_from_error (error_value, false);
  jerry_value_t err_str_val = jerry_value_to_string (error_value);
  jerry_size_t err_str_size = jerry_get_string_size (err_str_val);
  jerry_char_t err_str_buf[256];

  jerry_release_value (error_value);
  err_str_size = jerry_string_to_char_buffer (err_str_val, err_str_buf, err_str_size);
  if (err_str_size >= 242) //256-14
  {
    err_str_buf[241] = 0;
  }
  else
  {
    err_str_buf[err_str_size] = 0;
  }
  jerry_port_log (JERRY_LOG_LEVEL_ERROR, "Script Error: %s\n", err_str_buf);
  jerry_release_value (err_str_val);
} /* print_unhandled_exception */

void runtime_run_main() {
  uint32_t size = flash_get_data_size();
  if (size > 0) {
    uint8_t *script = flash_get_data();
    jerry_value_t parsed_code = jerry_parse (NULL, 0, script, size, JERRY_PARSE_STRICT_MODE);
    if (!jerry_value_is_error (parsed_code))
    {
      jerry_value_t ret_value = jerry_run (parsed_code);
      jerry_release_value (ret_value);
    } else {
      if (jerry_value_is_error (parsed_code))
      {
        print_unhandled_exception (parsed_code);
      }
    }
    jerry_release_value (parsed_code);  
  }
}
