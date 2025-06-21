/* Copyright (c) 2017-2020 Kaluma
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

#include "jerryscript.h"
#include "jerryxx.h"
#include "module_url.h"
#include "url_magic_strings.h"

static int fndch(const char *buf, char ch, int start, int end) {
  for (int i = start; i <= end; i++) {
    if (buf[i] == ch) {
      return i;
    }
  }
  return -1;
}

JERRYXX_FUN(url_ctor_fn) {
  JERRYXX_CHECK_ARG_STRING_OPT(0, "input")
  int protocol_s = -1;
  int protocol_e = -1;
  int origin_s = -1;
  int origin_e = -1;
  int path_s = -1;
  int path_e = -1;
  int auth_s = -1;
  int auth_e = -1;
  int host_s = -1;
  int host_e = -1;
  int user_s = -1;
  int user_e = -1;
  int pass_s = -1;
  int pass_e = -1;
  int hostname_s = -1;
  int hostname_e = -1;
  int port_s = -1;
  int port_e = -1;
  int pathname_s = -1;
  int pathname_e = -1;
  int hash_s = -1;
  int hash_e = -1;
  int search_s = -1;
  int search_e = -1;
  if (args_cnt > 0) {
    JERRYXX_GET_ARG_STRING_AS_CHAR(0, input)
    // extract protocol from href
    for (int i = 0; i <= input_sz - 3; i++) {
      if (input[i] == ':' && input[i + 1] == '/' && input[i + 2] == '/') {
        protocol_s = 0;
        protocol_e = i;
        break;
      }
    }
    if (protocol_e > 0) {
      origin_s = protocol_e + 3;  // exclude protocol part
      origin_e = input_sz - 1;
      // split origin + path
      int p1 = fndch(input, '/', origin_s, origin_e);
      if (p1 < 0) p1 = fndch(input, '?', origin_s, origin_e);
      if (p1 < 0) p1 = fndch(input, '#', origin_s, origin_e);
      if (p1 >= 0) {
        origin_e = p1 - 1;
        path_s = p1;
        path_e = input_sz - 1;
      }
      if (origin_e >= 0) {
        // split auth + host
        host_s = origin_s;
        host_e = origin_e;
        int p2 = fndch(input, '@', origin_s, origin_e);
        if (p2 >= 0) {
          auth_s = origin_s;
          auth_e = p2 - 1;
          host_s = p2 + 1;
          // split username + password
          user_s = auth_s;
          user_e = auth_e;
          int p3 = fndch(input, ':', auth_s, auth_e);
          if (p3 >= 0) {
            pass_s = p3 + 1;
            pass_e = user_e;
            user_e = p3 - 1;
          }
        }
        // split hostname + port
        hostname_s = host_s;
        hostname_e = host_e;
        int p4 = fndch(input, ':', host_s, host_e);
        if (p4 >= 0) {
          port_s = p4 + 1;
          port_e = host_e;
          hostname_e = p4 - 1;
        }
      }
      if (path_s >= 0) {
        // extract hash from path
        int p5 = fndch(input, '#', path_s, path_e);
        if (p5 >= 0) {
          hash_s = p5 + 1;
          hash_e = path_e;
          path_e = p5 - 1;
        }
        // extract search from path
        int p6 = fndch(input, '?', path_s, path_e);
        if (p6 >= 0) {
          search_s = p6;
          search_e = path_e;
          path_e = p6 - 1;
        }
        // extract pathname
        if (input[path_s] == '/') {
          pathname_s = path_s;
          pathname_e = path_e;
        }
      }
    } else {
      return jerry_error_sz(JERRY_ERROR_TYPE,
                                "Invalid URL");
    }
    // create properties
    jerry_value_t protocol;
    if (protocol_s >= 0) {
      protocol =
          jerry_string_sz((const char *)input + protocol_s,
                                 protocol_e - protocol_s + 1);
    } else {
      protocol = jerry_string_sz((const char *)"");
    }
    jerry_value_t username;
    if (user_s >= 0) {
      username = jerry_string_sz((const char *)input + user_s,
                                        user_e - user_s + 1);
    } else {
      username = jerry_string_sz((const char *)"");
    }
    jerry_value_t password;
    if (pass_s >= 0) {
      password = jerry_string_sz((const char *)input + pass_s,
                                        pass_e - pass_s + 1);
    } else {
      password = jerry_string_sz((const char *)"");
    }
    jerry_value_t hostname;
    if (hostname_s >= 0) {
      hostname =
          jerry_string_sz((const char *)input + hostname_s,
                                 hostname_e - hostname_s + 1);
    } else {
      hostname = jerry_string_sz((const char *)"");
    }
    jerry_value_t port;
    if (port_s >= 0) {
      port = jerry_string_sz((const char *)input + port_s,
                                    port_e - port_s + 1);
    } else {
      port = jerry_string_sz((const char *)"");
    }
    jerry_value_t pathname;
    if (pathname_s >= 0) {
      pathname =
          jerry_string_sz((const char *)input + pathname_s,
                                 pathname_e - pathname_s + 1);
    } else {
      pathname = jerry_string_sz((const char *)"");
    }
    jerry_value_t search;
    if (search_s >= 0) {
      search = jerry_string_sz((const char *)input + search_s,
                                      search_e - search_s + 1);
    } else {
      search = jerry_string_sz((const char *)"");
    }
    jerry_value_t hash;
    if (hash_s >= 0) {
      hash = jerry_string_sz((const char *)input + hash_s,
                                    hash_e - hash_s + 1);
    } else {
      hash = jerry_string_sz((const char *)"");
    }
    jerryxx_set_property(JERRYXX_GET_THIS, MSTR_URL_PROTOCOL, protocol);
    jerryxx_set_property(JERRYXX_GET_THIS, MSTR_URL_USERNAME, username);
    jerryxx_set_property(JERRYXX_GET_THIS, MSTR_URL_PASSWORD, password);
    jerryxx_set_property(JERRYXX_GET_THIS, MSTR_URL_HOSTNAME, hostname);
    jerryxx_set_property(JERRYXX_GET_THIS, MSTR_URL_PORT, port);
    jerryxx_set_property(JERRYXX_GET_THIS, MSTR_URL_PATHNAME, pathname);
    jerryxx_set_property(JERRYXX_GET_THIS, MSTR_URL_SEARCH, search);
    jerryxx_set_property(JERRYXX_GET_THIS, MSTR_URL_HASH, hash);
    jerry_value_free(protocol);
    jerry_value_free(username);
    jerry_value_free(password);
    jerry_value_free(hostname);
    jerry_value_free(port);
    jerry_value_free(pathname);
    jerry_value_free(search);
    jerry_value_free(hash);
  }
  return jerry_undefined();
}

jerry_value_t module_url_init() {
  /* URL class */
  jerry_value_t url_ctor = jerry_function_external(url_ctor_fn);
  jerry_value_t prototype = jerry_object();
  jerryxx_set_property(url_ctor, "prototype", prototype);
  jerry_value_free(prototype);
  // jerryxx_set_property_function(prototype, MSTR_PWM_START, pwm_start_fn);
  // jerryxx_set_property_function(prototype, MSTR_PWM_STOP, pwm_stop_fn);
  // jerryxx_set_property_function(prototype, MSTR_PWM_GET_FREQUENCY,
  // pwm_get_frequency_fn); jerryxx_set_property_function(prototype,
  // MSTR_PWM_SET_FREQUENCY, pwm_set_frequency_fn);
  // jerryxx_set_property_function(prototype, MSTR_PWM_GET_DUTY,
  // pwm_get_duty_fn); jerryxx_set_property_function(prototype,
  // MSTR_PWM_SET_DUTY, pwm_set_duty_fn);
  // jerryxx_set_property_function(prototype, MSTR_PWM_CLOSE, pwm_close_fn);

  /* url module exports */
  jerry_value_t exports = jerry_object();
  jerryxx_set_property(exports, MSTR_URL_URL, url_ctor);
  jerry_value_free(url_ctor);

  return exports;
}
