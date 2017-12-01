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

#include "jerryscript.h"
#include "jerryscript-ext/handler.h"

#include "runtime.h"
#include "global.h"
#include "kameleon_js.h"

#include "repl.h"

static jerry_value_t
process_get_native_module(const jerry_value_t func_value, /**< function object */
             const jerry_value_t this_val, /**< this arg */
             const jerry_value_t args_p[], /**< function arguments */
             const jerry_length_t args_cnt) /**< number of function arguments */
{
  /* Get module name */
  jerry_size_t module_name_sz = jerry_get_string_size(args_p[0]);
  jerry_char_t module_name[module_name_sz + 1];
  jerry_string_to_char_buffer(args_p[0], module_name, module_name_sz);
  module_name[module_name_sz] = '\0';

  /* Find and return corresponding module */
  for (int i = 0; i < native_modules_length; i++) {
    if (strcmp(native_modules[i].name, module_name) == 0) {
      jerry_value_t fn = jerry_exec_snapshot(native_modules[i].code, native_modules[i].size, true);
      return fn;
    }
  }

  /* If no corresponding module, return undefined */
  return jerry_create_undefined();
}

static void global_process_init() {
  jerry_value_t process_object = jerry_create_object();

  /* Add `process.native_modules` property */
  jerry_value_t array_native_modules = jerry_create_array(native_modules_length);
  for (int i = 0; i < native_modules_length; i++) {
    jerry_value_t value = jerry_create_string((const jerry_char_t *) native_modules[i].name);
    jerry_value_t ret = jerry_set_property_by_index(array_native_modules, i, value);
    jerry_release_value(ret);
    jerry_release_value(value);
  }
  jerry_value_t prop_native_modules = jerry_create_string((const jerry_char_t *) "native_modules");
  jerry_set_property(process_object, prop_native_modules, array_native_modules);
  jerry_release_value(prop_native_modules);
  jerry_release_value(array_native_modules);

  /* Add `process.getNativeModule` property */
  jerry_value_t get_native_module_fn = jerry_create_external_function(process_get_native_module);
  jerry_value_t get_native_module_prop = jerry_create_string((const jerry_char_t *) "getNativeModule");
  jerry_set_property (process_object, get_native_module_prop, get_native_module_fn);
  jerry_release_value (get_native_module_prop);
  jerry_release_value(get_native_module_fn);

  /* Register 'process' object to global */
  jerry_value_t global_object = jerry_get_global_object();
  jerry_value_t process_prop = jerry_create_string((const jerry_char_t *) "process");
  jerry_set_property(global_object, process_prop, process_object);
  jerry_release_value(process_prop);
  jerry_release_value(process_object);
  jerry_release_value(global_object);
}

static void register_global_objects() {
  jerry_value_t global_object = jerry_get_global_object ();
  jerry_value_t prop_name = jerry_create_string ((const jerry_char_t *) "global");
  jerry_set_property (global_object, prop_name, global_object);
  jerry_release_value (prop_name);
  jerry_release_value (global_object);
}

static void register_global_constants() {

}

static void register_global_functions() {
  jerryx_handler_register_global ((const jerry_char_t *) "print", jerryx_handler_print);
}

static void run_startup_module() {
  jerry_value_t res = jerry_exec_snapshot(module_startup_code, module_startup_size, false);
  jerry_value_t this_val = jerry_create_undefined ();
  jerry_value_t ret_val = jerry_call_function (res, this_val, NULL, 0);
  jerry_release_value (ret_val);
  jerry_release_value (this_val);
  jerry_release_value (res);  
}

void global_init() {
  global_process_init();
  register_global_objects();
  register_global_constants();
  register_global_functions();
  run_startup_module();
}
