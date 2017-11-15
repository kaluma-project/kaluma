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

#include "io.h"
#include "repl.h"

#define CONFIG_KAMELEON_VERSION "0.1.0"

/* Forward declarations */

static void repl_putc(char ch);
static void default_input_handler(repl_state_t *state, char ch);

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
  io_tty_read_start(&tty, repl_putc);
  state.mode = REPL_MODE_NORMAL;
  state.echo = true;
  state.buffer_length = 0;
  state.position = 0;
  state.escape_length = 0;
  state.history_size = 0;
  state.history_position = 0;
  state.input_handler = &default_input_handler;
  tty_printf("\33[2K\r");
  tty_printf("/---------------------------\\\n");
  tty_printf("|                  ____     |\n");
  tty_printf("|     /----_______/    \\    |\n");
  tty_printf("|    /               O  \\   |\n");
  tty_printf("|   /               _____\\  |\n");
  tty_printf("|  |  /------__ ___ ____/   |\n");
  tty_printf("|  | | /``\\   //   \\\\       |\n");
  tty_printf("|  | \\ @`\\ \\  W     W       |\n");
  tty_printf("|   \\ \\__/ / ***************|\n");
  tty_printf("|    \\____/     ************|\n");
  tty_printf("|                       ****|\n");
  tty_printf("\\---------------------------/\n");
  tty_printf("\n");
  tty_printf("Welcome to Kameleon!\n");
  tty_printf("%s %s\n", "Version:", CONFIG_KAMELEON_VERSION);
  tty_printf("For more info: http://kameleon.io\n");
  tty_printf("\n");
  repl_prompt();
}

/**
 * Inject a char to REPL
 */
static void repl_putc(char ch) {
  // ...
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
          // fflush(stdout);
        }
      }
      break;
    case 0x1b: /* escape char */
      state.mode = REPL_MODE_ESCAPE;
      state.escape_length = 0;
      tty_printf("\033[s"); // save current cursor pos
      // fflush(stdout);
      break;
    default:
      // check buffer overflow
      if (state.buffer_length < (MAX_BUFFER_LENGTH - 1)) {
        if (state.position == state.buffer_length) {
          state.buffer[state.position] = ch;
          state.buffer_length++;
          state.position++;
          if (state.echo) {
            tty_printf("%c", ch);
            // fflush(stdout);
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
            // fflush(stdout);
          }
        }
      } else {
        repl_error("%s\n", "REPL buffer overflow");
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
        // fflush(stdout);
        state.history_position--;
        char *cmd = state.history[state.history_position];
        tty_printf("\33[2K\r> %s", cmd);
        // fflush(stdout);
        strcpy(state.buffer, cmd);
        state.buffer_length = strlen(cmd);
        state.position = state.buffer_length;
      }

    // down key
    } else if (state.escape_length == 2 && state.escape[0] == 0x5b && state.escape[1] == 0x42) {
      tty_printf("\033[u"); // restore cursor position
      if (state.history_position == state.history_size) {
        // do nothing
      } else if (state.history_position == (state.history_size - 1)) {
        state.history_position++;
        tty_printf("\33[2K\r> ");
        // fflush(stdout);
        state.buffer_length = 0;
        state.position = 0;
      } else {
        state.history_position++;
        char *cmd = state.history[state.history_position];
        tty_printf("\33[2K\r> %s", cmd);
        // fflush(stdout);
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
        // fflush(stdout);
      }

    // right key
    } else if (state.escape_length == 2 && state.escape[0] == 0x5b && state.escape[1] == 0x43) {
      if (state.position >= state.buffer_length) {
        tty_printf("\033[u"); // restore cursor position
      } else {
        state.position++;
        tty_printf("\33[C");
        // fflush(stdout);
      }

    // Run original escape sequence
    } else {
      tty_putc('\033');
      for (int i = 0; i < state.escape_length; i++) {
        tty_putc(state.escape[i]);
      }
      // fflush(stdout);
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