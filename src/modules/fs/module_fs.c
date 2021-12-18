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

#include <stdlib.h>

#include "board.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "repl.h"
#include "tty.h"

static void cmd_ls(km_repl_state_t *state, char *arg) {
  km_repl_printf(".ls command ... \r\n");
}

static void cmd_pwd(km_repl_state_t *state, char *arg) {
  km_repl_printf(".pwd command ... \r\n");
}

static void cmd_cd(km_repl_state_t *state, char *arg) {
  km_repl_printf(".cd command .... \r\n");
}

/**
 * Initialize 'fs' module
 */
jerry_value_t module_fs_init() {
  // km_repl_register_command(".ls", "List files", cmd_ls);
  // km_repl_register_command(".pwd", "Print current directory", cmd_pwd);
  // km_repl_register_command(".cd", "Change current directory", cmd_cd);
  return jerry_create_undefined();
}
