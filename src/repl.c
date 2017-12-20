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
#include "flash.h"
#include "repl.h"
#include "runtime.h"
#include "jerryscript.h"
#include "utils.h"

#define CONFIG_KAMELEON_VERSION "0.1.0"

// --------------------------------------------------------------------------
// FORWARD DECLARATIONS
// --------------------------------------------------------------------------

static void cmd_echo(repl_state_t *state, char *arg);
static void cmd_flash(repl_state_t *state, char *arg);

// --------------------------------------------------------------------------
// PRIVATE VARIABLES
// --------------------------------------------------------------------------

/**
 * TTY handle for REPL
 */
static io_tty_handle_t tty;

/**
 * REPL state
 */
static repl_state_t state;

// --------------------------------------------------------------------------
// PRIVATE FUNCTIONS
// --------------------------------------------------------------------------

/**
 * Inject a char to REPL
 */
static void tty_read_cb(char ch) {
  if (state.handler != NULL) {
    (*state.handler)(&state, ch); /* call handler */
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

/**
 * Run the REPL command in the buffer
 */
static void run_command() {
  if (state.buffer_length > 0) {

    /* copy buffer to data */
    char *data = malloc(state.buffer_length + 1);
    state.buffer[state.buffer_length] = '\0';
    strcpy(data, state.buffer);
    state.buffer_length = 0;
    state.position = 0;

    /* push to history */
    history_push(data);

    /* tokenize command */
    char *tokenv[5];
    unsigned int tokenc = 0;
    tokenv[0] = strtok(state.buffer, " ");
    while (tokenv[tokenc] != NULL && tokenc < 5) {
      tokenc++;
      tokenv[tokenc] = strtok (NULL, " ");
    }

    /* run command */
    if (strcmp(tokenv[0], ".echo") == 0) {
      cmd_echo(&state, tokenv[1]);
    } else if (strcmp(tokenv[0], ".clear") == 0) {
      // TODO: cmd_clear();
    } else if (strcmp(tokenv[0], ".flash") == 0) {
      cmd_flash(&state, tokenv[1]);
    } else if (strcmp(tokenv[0], ".load") == 0) {
      // TODO: cmd_load(&state, tokenv[1]);
    } else { /* unknown command */
      repl_print_begin(REPL_OUTPUT_ERROR);
      repl_printf("Unknown command: %s\r\n", tokenv[0]);
      repl_print_end();
    }
  } else {
    repl_print_begin(REPL_OUTPUT_LOG);
    repl_print_end();    
  }
}

/**
 * Evaluate JS code in the buffer
 */
static void run_code() {
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
      repl_print_begin(REPL_OUTPUT_ERROR);
      repl_printf("%s\r\n", "Syntax error");
      repl_print_end();
    } else {
      jerry_value_t ret_value = jerry_run(parsed_code);
      if (jerry_value_has_error_flag(ret_value)) {
        repl_print_begin(REPL_OUTPUT_ERROR);
        repl_printf("Error\r\n");
        repl_print_end();
      } else {
        repl_print_begin(REPL_OUTPUT_INFO);
        repl_print_value(ret_value);
        repl_printf("\r\n");
        repl_print_end();
      }
      jerry_release_value(ret_value);
    }
    jerry_release_value(parsed_code);
  } else {
    repl_print_begin(REPL_OUTPUT_LOG);
    repl_print_end();
  }
}

/**
 * Handler for normal mode
 */
static void handle_normal(char ch) {
  switch (ch) {
    case '\r': /* carrage return */
      if (state.echo) {
        repl_printf("\r\n");
      }
      if (state.buffer_length > 0 && state.buffer[0] == '.') {
        run_command();
      } else {
        run_code();
      }
      break;
    case 0x08: /* backspace */
    case 0x7f: /* also backspace in some terminal */
      if (state.buffer_length > 0) {
        state.buffer_length--;
        state.buffer[state.buffer_length] = '\0';
        state.position--;
        if (state.echo) {
          repl_printf("\033[D\033[K");
        }
      }
      break;
    case 0x1b: /* escape char */
      state.mode = REPL_MODE_ESCAPE;
      state.escape_length = 0;
      repl_printf("\033[s"); // save current cursor pos
      break;
    default:
      // check buffer overflow
      if (state.buffer_length < (MAX_BUFFER_LENGTH - 1)) {
        if (state.position == state.buffer_length) {
          state.buffer[state.position] = ch;
          state.buffer_length++;
          state.position++;
          if (state.echo) {
            repl_putc(ch);
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
            repl_printf("\r> %s\033[%dG", state.buffer, state.position + 3);
          }
        }
      } else {
        repl_print_begin(REPL_OUTPUT_ERROR);
        repl_printf("%s\r\n", "REPL buffer overflow");
        repl_print_end();
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

    /* up key */
    if (state.escape_length == 2 && state.escape[0] == 0x5b && state.escape[1] == 0x41) {
      if (state.history_position > 0) {
        repl_printf("\033[u"); /* restore cursor position */
        state.history_position--;
        char *cmd = state.history[state.history_position];
        repl_printf("\33[2K\r> %s", cmd);
        strcpy(state.buffer, cmd);
        state.buffer_length = strlen(cmd);
        state.position = state.buffer_length;
      }

    /* down key */
    } else if (state.escape_length == 2 && state.escape[0] == 0x5b && state.escape[1] == 0x42) {
      repl_printf("\033[u"); // restore cursor position
      if (state.history_position == state.history_size) {
        /* do nothing */
      } else if (state.history_position == (state.history_size - 1)) {
        state.history_position++;
        repl_printf("\33[2K\r> ");
        state.buffer_length = 0;
        state.position = 0;
      } else {
        state.history_position++;
        char *cmd = state.history[state.history_position];
        repl_printf("\33[2K\r> %s", cmd);
        strcpy(state.buffer, cmd);
        state.buffer_length = strlen(cmd);
        state.position = state.buffer_length;
      }

    /* left key */
    } else if (state.escape_length == 2 && state.escape[0] == 0x5b && state.escape[1] == 0x44) {
      if (state.position == 0) {
        repl_printf("\033[u"); // restore cursor position
      } else {
        state.position--;
        repl_printf("\33[D");
      }

    /* right key */
    } else if (state.escape_length == 2 && state.escape[0] == 0x5b && state.escape[1] == 0x43) {
      if (state.position >= state.buffer_length) {
        repl_printf("\033[u"); // restore cursor position
      } else {
        state.position++;
        repl_printf("\33[C");
      }

    // Run original escape sequence
    } else {
      repl_putc('\033');
      for (int i = 0; i < state.escape_length; i++) {
        repl_putc(state.escape[i]);
      }
    }
  }
}

/**
 * Default handler
 */
static void default_handler(repl_state_t *state, char ch) {
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
 * Change handler
 */
static void set_handler(repl_handler_t handler) {
  if (handler != NULL) {
    state.handler = handler;
  } else {
    state.handler = &default_handler;
  }
}

/**
 * .echo command
 */
static void cmd_echo(repl_state_t *state, char *arg) {
  if (strcmp(arg, "on") == 0) {
    state->echo = true;
  } else if (strcmp(arg, "off") == 0) {
    state->echo = false;
  }
}

/**
 * State for flash command
 */
static struct {
  char buffer[2];
  unsigned int buffer_length;
} cmd_flash_state;

/**
 * Handler for .flash command
 */
static void cmd_flash_handler(repl_state_t *state, char ch) {
  switch (ch) {
    case 0x1a: /* Ctrl+Z */
      flash_program_end();
      set_handler(NULL);
      repl_print_begin(REPL_OUTPUT_LOG);
      repl_printf("\r\n");
      repl_print_end();
      break;
    default:
      cmd_flash_state.buffer[cmd_flash_state.buffer_length] = ch;
      cmd_flash_state.buffer_length++;
      if (state->echo) {
        repl_putc(ch);
      }
      if (cmd_flash_state.buffer_length == 2) {
        if (state->echo) {
          repl_putc(' ');
        }
        uint8_t dat = hex2bin(cmd_flash_state.buffer);
        flash_status_t flash_status = flash_program(&dat, 1);
        if (FLASH_SUCCESS != flash_status) {
          repl_print_begin(REPL_OUTPUT_ERROR);
          repl_printf("%s\r\n", "Failed during data writing to file.");
          repl_print_end();
          flash_program_end();
          set_handler(NULL);
        }
        cmd_flash_state.buffer_length = 0;
      }
      break;
  }
}

/**
 * .flash command
 */
static void cmd_flash(repl_state_t *state, char *arg) {
  if (strcmp(arg, "-e") == 0) { /* erase flash */
    flash_clear();
    repl_print_begin(REPL_OUTPUT_LOG);
    repl_printf("Flash has erased\r\n");
    repl_print_end();
  } else if (strcmp(arg, "-c") == 0) { /* get checksum */
    uint32_t checksum = flash_get_checksum();
    repl_print_begin(REPL_OUTPUT_LOG);
    repl_printf("%u\r\n", checksum);
    repl_print_end();
  } else if (strcmp(arg, "-t") == 0) { /* get total size of flash */
    uint32_t size = flash_size();
    repl_print_begin(REPL_OUTPUT_LOG);
    repl_printf("%u\r\n", size);
    repl_print_end();
  } else if (strcmp(arg, "-s") == 0) { /* get data size in flash */
    uint32_t data_size = flash_get_data_size();
    repl_print_begin(REPL_OUTPUT_LOG);
    repl_printf("%u\r\n", data_size);
    repl_print_end();
  } else if (strcmp(arg, "-r") == 0) { /* read data */
    uint32_t sz = flash_get_data_size();
    uint8_t *ptr = flash_get_data();
    repl_print_begin(REPL_OUTPUT_LOG);
    for (int i = 0; i < sz; i++) {
      if (ptr[i] == '\n') { /* convert "\n" to "\r\n" */
        repl_putc('\r');
      }
      repl_putc(ptr[i]);
    }
    repl_printf("\r\n");
    repl_print_end();
  } else if (strcmp(arg, "-w") == 0) { /* write mode */
    flash_program_begin();
    set_handler(&cmd_flash_handler);
  } else {
    repl_print_begin(REPL_OUTPUT_LOG);
    repl_printf(".flash command options:\r\n");
    repl_printf("-w\tWrite data in hex format\r\n");
    repl_printf("-e\tErase the flash\r\n");
    repl_printf("-c\tGet checksum\r\n");
    repl_printf("-t\tGet total size of flash\r\n");
    repl_printf("-s\tGet data size in flash\r\n");
    repl_printf("-r\tRead data in textual format\r\n");
    repl_print_end();    
  }
}

// --------------------------------------------------------------------------
// PUBLIC FUNCTIONS
// --------------------------------------------------------------------------

/**
 * Initialize the REPL
 */
void repl_init() {
  io_tty_init(&tty);
  io_tty_read_start(&tty, tty_read_cb);
  state.mode = REPL_MODE_NORMAL;
  state.echo = true;
  state.buffer_length = 0;
  state.position = 0;
  state.escape_length = 0;
  state.history_size = 0;
  state.history_position = 0;
  state.handler = &default_handler;
  repl_print_begin(REPL_OUTPUT_LOG);
  repl_printf("/---------------------------\\\r\n");
  repl_printf("|                  ____     |\r\n");
  repl_printf("|     /----_______/    \\    |\r\n");
  repl_printf("|    /               O  \\   |\r\n");
  repl_printf("|   /               _____\\  |\r\n");
  repl_printf("|  |  /------__ ___ ____/   |\r\n");
  repl_printf("|  | | /``\\   //   \\\\       |\r\n");
  repl_printf("|  | \\ @`\\ \\  W     W       |\r\n");
  repl_printf("|   \\ \\__/ / ***************|\r\n");
  repl_printf("|    \\____/     ************|\r\n");
  repl_printf("|                       ****|\r\n");
  repl_printf("\\---------------------------/\r\n");
  repl_printf("\r\n");
  repl_printf("Welcome to Kameleon!\r\n");
  repl_printf("%s %s\r\n", "Version:", CONFIG_KAMELEON_VERSION);
  repl_printf("For more info: http://kameleon.io\r\n");
  repl_printf("\r\n");
  repl_print_end();
}

void repl_print_begin(repl_output_t output) {
  tty_printf("\33[2K\r"); /* set column to 0 */
  switch (output) {
    case REPL_OUTPUT_LOG:
      tty_printf("\33[0m"); /* set to normal color */
      break;
    case REPL_OUTPUT_INFO:
      tty_printf("\33[90m"); /* set to dark gray color */
      break;
    case REPL_OUTPUT_ERROR:
      tty_printf("\33[31m"); /* set to red color */
      break;
  }
}

void repl_print_end() {
  tty_printf("\33[0m"); // back to normal color
  if (state.echo) {
    state.buffer[state.buffer_length] = '\0';
    tty_printf("> %s", &state.buffer);
  }
}

