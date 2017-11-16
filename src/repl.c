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

#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "tty.h"
#include "repl.h"
#include "jerryscript.h"

#define CONFIG_KAMELEON_VERSION "0.1.0"

/* Forward declarations */

static void repl_getc(char ch);
static void default_input_handler(repl_state_t *state, char ch);
static void repl_prompt();

/**
 * TTY handle for REPL
 */
static io_tty_handle_t tty;

/**
 * REPL state
 */
static repl_state_t state;

/**
 * Initialize the REPL
 */
void repl_init() {
  io_tty_init(&tty);
  io_tty_read_start(&tty, repl_getc);

  jerry_init(JERRY_INIT_EMPTY); /* TODO: MOVE THIS TO OTHER PLACE */

  state.mode = REPL_MODE_NORMAL;
  state.echo = true;
  state.buffer_length = 0;
  state.position = 0;
  state.escape_length = 0;
  state.history_size = 0;
  state.history_position = 0;
  state.input_handler = &default_input_handler;
  tty_printf("\33[2K\r");
  tty_printf("/---------------------------\\\r\n");
  tty_printf("|                  ____     |\r\n");
  tty_printf("|     /----_______/    \\    |\r\n");
  tty_printf("|    /               O  \\   |\r\n");
  tty_printf("|   /               _____\\  |\r\n");
  tty_printf("|  |  /------__ ___ ____/   |\r\n");
  tty_printf("|  | | /``\\   //   \\\\       |\r\n");
  tty_printf("|  | \\ @`\\ \\  W     W       |\r\n");
  tty_printf("|   \\ \\__/ / ***************|\r\n");
  tty_printf("|    \\____/     ************|\r\n");
  tty_printf("|                       ****|\r\n");
  tty_printf("\\---------------------------/\r\n");
  tty_printf("\r\n");
  tty_printf("Welcome to Kameleon!\r\n");
  tty_printf("%s %s\r\n", "Version:", CONFIG_KAMELEON_VERSION);
  tty_printf("For more info: http://kameleon.io\r\n");
  tty_printf("\r\n");
  repl_prompt();
}

static void print_value (const jerry_value_t value) {
  if (jerry_value_has_error_flag(value)) {
    repl_error("%s\r\n", "Error.");
  } else {
    jerry_value_t str_value = jerry_value_to_string(value);

    /* Determining required buffer size */
    jerry_size_t req_sz = jerry_get_string_size (str_value);
    jerry_char_t str_buf_p[req_sz + 1];

    jerry_string_to_char_buffer (str_value, str_buf_p, req_sz);
    str_buf_p[req_sz] = '\0';
    if (jerry_value_is_string(value)) {
      repl_info("\"%s\"\r\n", (char *) str_buf_p);
    } else if (jerry_value_is_array(value)) {
      repl_info("[%s]\r\n", (char *) str_buf_p);
    } else {
      repl_info("%s\r\n", (char *) str_buf_p);
    }
  }
}

static void repl_prompt() {
  if (state.echo) {
    state.buffer[state.buffer_length] = '\0';
    tty_printf("> %s", &state.buffer);
  }
}

/**
 * Inject a char to REPL
 */
static void repl_getc(char ch) {
  if (state.input_handler != NULL) {
    (*state.input_handler)(&state, ch); /* call input handler */
  }
}

void repl_set_input_handler(repl_input_handler_t handler) {
  if (handler != NULL) {
    state.input_handler = handler;
  } else {
    state.input_handler = &default_input_handler;
  }
}

/**
 * Push a command to history
 */
static void history_push(char *cmd) {
  if (state.history_size < MAX_COMMAND_HISTORY) {
    state.history[state.history_size] = cmd;
    state.history_size++;
  } else {
    // free memory of history[0]
    free(state.history[0]);
    // Shift history array to left (e.g. 1 to 0, 2 to 1, ...)
    for (int i = 0; i < (state.history_size - 1); i++) {
      state.history[i] = state.history[i + 1];
    }
    // Put to the last of history
    state.history[state.history_size - 1] = cmd;
  }
  state.history_position = state.history_size;
}

static void run_command() {
  state.buffer_length = 0;
  state.position = 0;  
  repl_prompt();
}

static void eval_code() {
  if (state.buffer_length > 0) {

    /* copy buffer to data */
    char *data = malloc(state.buffer_length + 1);
    state.buffer[state.buffer_length] = '\0';
    strcpy(data, state.buffer);
    state.buffer_length = 0;
    state.position = 0;

    /* push to history */
    history_push(data);

    /* evaluate code */
    jerry_value_t parsed_code = jerry_parse((const jerry_char_t *) data, strlen(data), false);
    if (jerry_value_has_error_flag (parsed_code)) {
      repl_error("%s\r\n", "Syntax error");
    } else {
      jerry_value_t ret_value = jerry_run(parsed_code);
      print_value(ret_value);
      jerry_release_value(ret_value);
    }
    jerry_release_value(parsed_code);
  } else {
    repl_prompt();
  }
}

/**
 * Handler for normal mode
 */
static void handle_normal(char ch) {
  switch (ch) {
    case '\r': /* carrage return */
      if (state.echo) {
        tty_printf("\r\n");
      }
      if (state.buffer_length > 0 && state.buffer[0] == '.') {
        run_command();
      } else {
        eval_code();
      }
      break;
    case 0x08: /* backspace */
    case 0x7f: /* also backspace in some terminal */
      if (state.buffer_length > 0) {
        state.buffer_length--;
        state.buffer[state.buffer_length] = '\0';
        state.position--;
        if (state.echo) {
          tty_printf("\033[D\033[K");
        }
      }
      break;
    case 0x1b: /* escape char */
      state.mode = REPL_MODE_ESCAPE;
      state.escape_length = 0;
      tty_printf("\033[s"); // save current cursor pos
      break;
    default:
      // check buffer overflow
      if (state.buffer_length < (MAX_BUFFER_LENGTH - 1)) {
        if (state.position == state.buffer_length) {
          state.buffer[state.position] = ch;
          state.buffer_length++;
          state.position++;
          if (state.echo) {
            tty_putc(ch);
          }
        } else {
          for (int i = state.buffer_length; i > state.position; i--) {
            state.buffer[i] = state.buffer[i - 1];
          }
          state.buffer[state.position] = ch;
          state.buffer_length++;
          state.position++;
          state.buffer[state.buffer_length] = '\0';
          if (state.echo) {
            tty_printf("\r> %s\033[%dG", state.buffer, state.position + 3);
          }
        }
      } else {
        repl_error("%s\r\n", "REPL buffer overflow");
      }
      break;
  }
}

/**
 * Handle char in escape sequence
 */
static void handle_escape(char ch) {
  state.escape[state.escape_length] = ch;
  state.escape_length++;

  // if ch is last char (a-zA-Z) of escape sequence
  if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
    state.mode = REPL_MODE_NORMAL;

    // up key
    if (state.escape_length == 2 && state.escape[0] == 0x5b && state.escape[1] == 0x41) {
      if (state.history_position > 0) {
        tty_printf("\033[u"); // restore cursor position
        state.history_position--;
        char *cmd = state.history[state.history_position];
        tty_printf("\33[2K\r> %s", cmd);
        strcpy(state.buffer, cmd);
        state.buffer_length = strlen(cmd);
        state.position = state.buffer_length;
      }

    // down key
    } else if (state.escape_length == 2 && state.escape[0] == 0x5b && state.escape[1] == 0x42) {
      tty_printf("\033[u"); // restore cursor position
      if (state.history_position == state.history_size) {
        /* do nothing */
      } else if (state.history_position == (state.history_size - 1)) {
        state.history_position++;
        tty_printf("\33[2K\r> ");
        state.buffer_length = 0;
        state.position = 0;
      } else {
        state.history_position++;
        char *cmd = state.history[state.history_position];
        tty_printf("\33[2K\r> %s", cmd);
        strcpy(state.buffer, cmd);
        state.buffer_length = strlen(cmd);
        state.position = state.buffer_length;
      }

    // left key
    } else if (state.escape_length == 2 && state.escape[0] == 0x5b && state.escape[1] == 0x44) {
      if (state.position == 0) {
        tty_printf("\033[u"); // restore cursor position
      } else {
        state.position--;
        tty_printf("\33[D");
      }

    // right key
    } else if (state.escape_length == 2 && state.escape[0] == 0x5b && state.escape[1] == 0x43) {
      if (state.position >= state.buffer_length) {
        tty_printf("\033[u"); // restore cursor position
      } else {
        state.position++;
        tty_printf("\33[C");
      }

    // Run original escape sequence
    } else {
      tty_putc('\033');
      for (int i = 0; i < state.escape_length; i++) {
        tty_putc(state.escape[i]);
      }
    }
  }
}

/**
 * Default input handler
 */
static void default_input_handler(repl_state_t *state, char ch) {
  switch (state->mode) {
    case REPL_MODE_NORMAL:
      handle_normal(ch);
      break;
    case REPL_MODE_ESCAPE:
      handle_escape(ch);
      break;
  }
}

/**
 * Print a log string to the console
 */
void repl_log(const char *format, const char *str) {
  tty_printf("\33[2K\r"); // set column to 0
  tty_printf("\33[0m"); // set to normal color
  tty_printf(format, str);
  repl_prompt();
}

/**
 * Print a info string to the console.
 * Evaluated value is printed on console as an info.
 */
void repl_info(const char *format, const char *str) {
  tty_printf("\33[2K\r"); // set column to 0
  tty_printf("\33[90m"); // set to dark gray color
  tty_printf(format, str);
  tty_printf("\33[0m"); // back to normal color
  repl_prompt();
}

/**
 * Print an error string to the console
 */
void repl_error(const char *format, const char *str) {
  tty_printf("\33[2K\r"); // set column to 0
  tty_printf("\33[31m"); // red
  tty_printf(format, str);
  tty_printf("\33[0m"); // back to normal color
  repl_prompt();
}
