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
  jerry_value_t path = jerryxx_call_require("path");
  jerry_value_t cwd_js = jerryxx_call_method(fs, MSTR_FS_CWD, NULL, 0);
  if (jerry_value_is_error(cwd_js)) {
    jerryxx_print_error(cwd_js, true);
  } else {
    jerry_value_t args_js[1] = {cwd_js};
    jerry_value_t file_array_js =
        jerryxx_call_method(fs, MSTR_FS_READDIR, args_js, 1);
    if (jerry_value_is_error(file_array_js)) {
      jerryxx_print_error(file_array_js, true);
    } else {
      int len = jerry_array_length(file_array_js);
      for (int i = 0; i < len; i++) {
        jerry_value_t file_js = jerry_object_get_index(file_array_js, i);
        jerry_value_t join_args[2] = {cwd_js, file_js};
        jerry_value_t file_path_js =
            jerryxx_call_method(path, "join", join_args, 2);
        jerry_value_t stat_args[1] = {file_path_js};
        jerry_value_t file_stat_js =
            jerryxx_call_method(fs, MSTR_FS_STAT, stat_args, 1);
        jerry_value_t is_dir_js = jerryxx_call_method(
            file_stat_js, MSTR_FS_STATS_IS_DIRECTORY, NULL, 0);
        bool is_dir = jerry_value_is_true(is_dir_js);
        int file_size =
            jerryxx_get_property_number(file_stat_js, MSTR_FS_STATS_SIZE, 0);
        JERRYXX_GET_STRING_AS_CHAR(file_js, file_name);
        if (is_dir) {
          km_repl_printf("<dir>\t%s\r\n", file_name);
        } else {
          km_repl_printf("%u\t%s\r\n", file_size, file_name);
        }
        jerry_value_free(is_dir_js);
        jerry_value_free(file_stat_js);
        jerry_value_free(file_path_js);
        jerry_value_free(file_js);
      }
    }
    jerry_value_free(file_array_js);
  }
  jerry_value_free(cwd_js);
  jerry_value_free(path);
  jerry_value_free(fs);
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
  jerry_value_free(ret);
  jerry_value_free(fs);
}

static void cmd_cd(km_repl_state_t *state, char *arg) {
  jerry_value_t fs = jerryxx_call_require("fs");
  jerry_value_t path_js = jerry_string_sz((const char *)arg);
  jerry_value_t args_js[1] = {path_js};
  jerry_value_t ret = jerryxx_call_method(fs, MSTR_FS_CHDIR, args_js, 1);
  if (jerry_value_is_error(ret)) {
    jerryxx_print_error(ret, true);
  }
  jerry_value_free(ret);
  jerry_value_free(path_js);
  jerry_value_free(fs);
}

static void cmd_mkdir(km_repl_state_t *state, char *arg) {
  jerry_value_t fs = jerryxx_call_require("fs");
  jerry_value_t path_js = jerry_string_sz((const char *)arg);
  jerry_value_t args_js[1] = {path_js};
  jerry_value_t ret = jerryxx_call_method(fs, MSTR_FS_MKDIR, args_js, 1);
  if (jerry_value_is_error(ret)) {
    jerryxx_print_error(ret, true);
  }
  jerry_value_free(ret);
  jerry_value_free(path_js);
  jerry_value_free(fs);
}

static void cmd_rm(km_repl_state_t *state, char *arg) {
  jerry_value_t fs = jerryxx_call_require("fs");
  jerry_value_t path_js = jerry_string_sz((const char *)arg);
  jerry_value_t args_js[1] = {path_js};
  jerry_value_t ret = jerryxx_call_method(fs, MSTR_FS_RM, args_js, 1);
  if (jerry_value_is_error(ret)) {
    jerryxx_print_error(ret, true);
  }
  jerry_value_free(ret);
  jerry_value_free(path_js);
  jerry_value_free(fs);
}

static void cmd_cat(km_repl_state_t *state, char *arg) {
  jerry_value_t fs = jerryxx_call_require("fs");
  jerry_value_t path_js = jerry_string_sz((const char *)arg);
  jerry_value_t args_js[1] = {path_js};
  jerry_value_t fd = jerryxx_call_method(fs, MSTR_FS_OPEN, args_js, 1);
  int buf_size = 128;
  jerry_value_t buf_js =
      jerry_typedarray(JERRY_TYPEDARRAY_UINT8, buf_size);
  jerry_value_t buf_size_js = jerry_number(buf_size);
  uint8_t *buf = jerryxx_get_typedarray_buffer(buf_js);
  int pos = 0;
  int read_bytes = 0;
  do {
    jerry_value_t pos_js = jerry_number(pos);
    jerry_value_t read_args_js[5] = {fd, buf_js, 0, buf_size_js, pos_js};
    jerry_value_t ret = jerryxx_call_method(fs, MSTR_FS_READ, read_args_js, 5);
    read_bytes = jerry_value_as_number(ret);
    for (int i = 0; i < read_bytes; i++) {
      km_tty_putc(buf[i]);
    }
    pos += read_bytes;
    jerry_value_free(ret);
    jerry_value_free(pos_js);
  } while (read_bytes == buf_size);
  km_tty_printf("\r\n");
  jerry_value_free(buf_size_js);
  jerry_value_free(buf_js);
  jerry_value_free(fd);
  jerry_value_free(path_js);
  jerry_value_free(fs);
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
  km_repl_register_command(".cat", "Print the content of file", cmd_cat);
  // km_repl_register_command(".cp", "Copy file", cmd_cp);
  // km_repl_register_command(".ftr", "File transfer", cmd_ftr);
  return jerry_undefined();
}
