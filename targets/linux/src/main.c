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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "board.h"
#include "gpio.h"
#include "io.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "repl.h"
#include "runtime.h"
#include "system.h"
#include "tty.h"

int main(int argc, char* argv[]) {
  bool en_repl = false;
  if ((argc == 1) || ((argc > 2) && (strcmp(argv[2], "-i") == 0))) {
    en_repl = true;
  }
  km_system_init();
  km_tty_init();
  km_io_init();
  km_repl_init(en_repl);
  km_runtime_init(false, false);

  // read file
  if (argc > 1) {
    FILE* f;
    f = fopen(argv[1], "r");
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* script = malloc(size + 1);
    fread(script, 1, size, f);
    script[size] = '\0';
    fclose(f);

    // printf("%s\r\n", buffer);
    jerry_value_t parsed_code = jerry_parse(NULL, 0, (jerry_char_t*)script,
                                            size, JERRY_PARSE_STRICT_MODE);
    if (!jerry_value_is_error(parsed_code)) {
      jerry_value_t ret_value = jerry_run(parsed_code);
      if (jerry_value_is_error(ret_value)) {
        jerryxx_print_error(ret_value, true);
        // km_runtime_cleanup();
        // km_runtime_init(false, false);
        return 0;
      }
      jerry_release_value(ret_value);
    } else {
      jerryxx_print_error(parsed_code, true);
    }
    jerry_release_value(parsed_code);
  }

  km_io_run(en_repl);
}
