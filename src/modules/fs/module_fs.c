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
#include "fs_magic_strings.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "repl.h"
#include "tty.h"

static void cmd_ls(km_repl_state_t *state, char *arg) {
  jerry_value_t fs = jerryxx_call_require("fs");
  jerry_value_t cwd_js = jerryxx_call_method(fs, MSTR_FS_CWD, NULL, 0);
  if (jerry_value_is_error(cwd_js)) {
    jerryxx_print_error(cwd_js, true);
  } else {
    jerry_value_t args_js[1] = {cwd_js};
    jerry_value_t file_array_js =
        jerryxx_call_method(fs, MSTR_FS_READDIR_SYNC, args_js, 1);
    if (jerry_value_is_error(file_array_js)) {
      jerryxx_print_error(file_array_js, true);
    } else {
      int len = jerry_get_array_length(file_array_js);
      for (int i = 0; i < len; i++) {
        jerry_value_t file_js = jerry_get_property_by_index(file_array_js, i);
        JERRYXX_GET_STRING_AS_CHAR(file_js, file_name);
        km_repl_printf("%s\r\n", file_name);
        jerry_release_value(file_js);
      }
    }
    jerry_release_value(file_array_js);
  }
  jerry_release_value(cwd_js);
  jerry_release_value(fs);
}

static void cmd_pwd(km_repl_state_t *state, char *arg) {
  jerry_value_t fs = jerryxx_call_require("fs");
  jerry_value_t ret = jerryxx_call_method(fs, MSTR_FS_CWD, NULL, 0);
  if (jerry_value_is_error(ret)) {
    jerryxx_print_error(ret, true);
  } else {
    JERRYXX_GET_STRING_AS_CHAR(ret, cwd);
    km_repl_printf("%s\r\n", cwd);
  }
  jerry_release_value(ret);
  jerry_release_value(fs);
}

static void cmd_cd(km_repl_state_t *state, char *arg) {
  jerry_value_t fs = jerryxx_call_require("fs");
  jerry_value_t path_js = jerry_create_string((const jerry_char_t *)arg);
  jerry_value_t args_js[1] = {path_js};
  jerry_value_t ret = jerryxx_call_method(fs, MSTR_FS_CHDIR, args_js, 1);
  if (jerry_value_is_error(ret)) {
    jerryxx_print_error(ret, true);
  }
  jerry_release_value(ret);
  jerry_release_value(path_js);
  jerry_release_value(fs);
}

static void cmd_mkdir(km_repl_state_t *state, char *arg) {
  jerry_value_t fs = jerryxx_call_require("fs");
  jerry_value_t path_js = jerry_create_string((const jerry_char_t *)arg);
  jerry_value_t args_js[1] = {path_js};
  jerry_value_t ret = jerryxx_call_method(fs, MSTR_FS_MKDIR_SYNC, args_js, 1);
  if (jerry_value_is_error(ret)) {
    jerryxx_print_error(ret, true);
  }
  jerry_release_value(ret);
  jerry_release_value(path_js);
  jerry_release_value(fs);
}

static void cmd_rm(km_repl_state_t *state, char *arg) {
  jerry_value_t fs = jerryxx_call_require("fs");
  jerry_value_t path_js = jerry_create_string((const jerry_char_t *)arg);
  jerry_value_t args_js[1] = {path_js};
  jerry_value_t ret = jerryxx_call_method(fs, MSTR_FS_RM_SYNC, args_js, 1);
  if (jerry_value_is_error(ret)) {
    jerryxx_print_error(ret, true);
  }
  jerry_release_value(ret);
  jerry_release_value(path_js);
  jerry_release_value(fs);
}

/*
static void cmd_cp(km_repl_state_t *state, char *arg) {
  // copy file (or entire directory)
}

static void cmd_ftr(km_repl_state_t *state, char *arg) {
  // transfer a file via ymodem
}
*/

/**
 * Initialize 'fs' module
 */
jerry_value_t module_fs_init() {
  km_repl_register_command(".ls", "List files", cmd_ls);
  km_repl_register_command(".pwd", "Print current directory", cmd_pwd);
  km_repl_register_command(".cd", "Change current directory", cmd_cd);
  km_repl_register_command(".mkdir", "Create directory", cmd_mkdir);
  km_repl_register_command(".rm", "Remove file or directory", cmd_rm);
  // km_repl_register_command(".cp", "Copy file", cmd_cp);
  // km_repl_register_command(".ftr", "File transfer", cmd_ftr);
  return jerry_create_undefined();
}
