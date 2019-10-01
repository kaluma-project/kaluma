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
#include "system.h"
#include "jerryscript.h"
#include "utils.h"
#include "kameleon_config.h"
#include "ymodem.h"

// --------------------------------------------------------------------------
// FORWARD DECLARATIONS
// --------------------------------------------------------------------------

static void cmd_echo(repl_state_t *state, char *arg);
static void cmd_reset(repl_state_t *state);
static void cmd_flash(repl_state_t *state, char *arg);
static void cmd_load(repl_state_t *state);
static void cmd_mem(repl_state_t *state);
static void cmd_gc(repl_state_t *state);
static void cmd_firmup(repl_state_t *state);
static void cmd_hi(repl_state_t *state);
static void cmd_help(repl_state_t *state);

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
static void tty_read_cb(uint8_t *buf, size_t len) {
  if (state.handler != NULL) {
    (*state.handler)(&state, buf, len); /* call handler */
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
    } else if (strcmp(tokenv[0], ".reset") == 0) {
      cmd_reset(&state);
    } else if (strcmp(tokenv[0], ".flash") == 0) {
      cmd_flash(&state, tokenv[1]);
    } else if (strcmp(tokenv[0], ".load") == 0) {
      cmd_load(&state);
    } else if (strcmp(tokenv[0], ".mem") == 0) {
      cmd_mem(&state);
    } else if (strcmp(tokenv[0], ".gc") == 0) {
      cmd_gc(&state);
    } else if (strcmp(tokenv[0], ".firmup") == 0) {
      cmd_firmup(&state);
    } else if (strcmp(tokenv[0], ".hi") == 0) {
      cmd_hi(&state);
    } else if (strcmp(tokenv[0], ".help") == 0) {
      cmd_help(&state);
    } else { /* unknown command */
      repl_set_output(REPL_OUTPUT_ERROR);
      repl_printf("Unknown command: %s\r\n", tokenv[0]);
      repl_set_output(REPL_OUTPUT_NORMAL);
    }
  } else {
    repl_set_output(REPL_OUTPUT_NORMAL);
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
    jerry_value_t parsed_code = jerry_parse(NULL, 0, (const jerry_char_t *) data, strlen(data), JERRY_PARSE_STRICT_MODE);
    if (jerry_value_is_error(parsed_code)) {
      repl_set_output(REPL_OUTPUT_ERROR);
      jerry_value_t parse_err = jerry_get_value_from_error(parsed_code, false);
      repl_print_value(parse_err);
      repl_println();
      jerry_release_value(parse_err);
      repl_set_output(REPL_OUTPUT_NORMAL);
    } else {
      jerry_value_t ret_value = jerry_run(parsed_code);
      if (jerry_value_is_error(ret_value)) {
        repl_set_output(REPL_OUTPUT_ERROR);
        jerry_value_t err = jerry_get_value_from_error(ret_value, false);
        repl_print_value(err);
        repl_println();
        jerry_release_value(err);
        repl_set_output(REPL_OUTPUT_NORMAL);
      } else {
        repl_set_output(REPL_OUTPUT_INFO);
        repl_print_value(ret_value);
        repl_println();
        repl_set_output(REPL_OUTPUT_ERROR);
      }
      jerry_release_value(ret_value);
    }
    jerry_release_value(parsed_code);
  }
}

/**
 * Move cursor to state.position in consideration with state.width
 */
static void set_cursor_to_position() {
  int horz = (state.position + 2) % state.width;
  int vert = (state.position + 2) / state.width;
  if (horz > 0) {
    if (vert > 0) {
      repl_printf("\033[u\033[%dC\033[%dB", horz, vert);
    } else {
      repl_printf("\033[u\033[%dC", horz);
    }
  } else {
    if (vert > 0) {
      repl_printf("\033[u\033[%dB", vert);
    } else {
      repl_printf("\033[u");
    }
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
      repl_print_prompt();      
      break;
    case 0x08: /* backspace */
    case 0x7f: /* also backspace in some terminal */
      if (state.buffer_length > 0 && state.position > 0) {
        state.position--;
        for (int i = state.position; i < state.buffer_length - 1; i++) {
          state.buffer[i] = state.buffer[i + 1];
        }
        state.buffer_length--;
        state.buffer[state.buffer_length] = '\0';
        if (state.echo) {
          repl_printf("\033[D\033[K\033[J");
          repl_printf("\033[u> %s", state.buffer);
          set_cursor_to_position();
        }
      }
      break;
    case 0x1b: /* escape char */
      state.mode = REPL_MODE_ESCAPE;
      state.escape_length = 0;
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
            repl_printf("\033[u> %s", state.buffer);
            set_cursor_to_position();
          }
        }
      } else {
        repl_set_output(REPL_OUTPUT_ERROR);
        repl_printf("%s\r\n", "REPL buffer overflow");
        repl_set_output(REPL_OUTPUT_NORMAL);
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
        state.history_position--;
        char *cmd = state.history[state.history_position];
        repl_printf("\33[2K\r> %s", cmd);
        strcpy(state.buffer, cmd);
        state.buffer_length = strlen(cmd);
        state.position = state.buffer_length;
      }

    /* down key */
    } else if (state.escape_length == 2 && state.escape[0] == 0x5b && state.escape[1] == 0x42) {
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
      if (state.position > 0) {
        state.position--;
        set_cursor_to_position();
      }

    /* right key */
    } else if (state.escape_length == 2 && state.escape[0] == 0x5b && state.escape[1] == 0x43) {
      if (state.position < state.buffer_length) {
        state.position++;
        set_cursor_to_position();
      }

    /* receive cursor position and update screen width */
    } else if (state.escape[state.escape_length - 1] == 'R') {
      int pos = 0;
      for (int i = 0; i < state.escape_length; i++) {
        if (state.escape[i] == ';') {
          pos = i + 1;
          break;
        }
      }
      state.escape[state.escape_length - 1] = '\0';
      state.width = atoi(state.escape + pos);

    // Run original escape sequence
    } else {
      repl_putc('\033');
      for (int i = 0; i < state.escape_length; i++) {
        repl_putc(state.escape[i]);
      }
    }
  } else if (ch == 0x7e) { /* special key */
    state.mode = REPL_MODE_NORMAL;

    /* delete key */
    if (state.escape_length == 3 && state.escape[0] == 0x5b && state.escape[1] == 0x33) {
      if (state.buffer_length > 0 && state.position < state.buffer_length) {
        for (int i = state.position; i < state.buffer_length; i++) {
          state.buffer[i] = state.buffer[i + 1];
        }
        state.buffer_length--;
        state.buffer[state.buffer_length] = '\0';
        if (state.echo) {
          repl_printf("\033[K\033[J");
          repl_printf("\033[u> %s", state.buffer);
          set_cursor_to_position();
        }
      }
    }
    state.escape_length = 0;
  }
}

/**
 * Default handler
 */
static void default_handler(repl_state_t *state, uint8_t *buf, size_t len) {
  for (int i = 0; i < len; i++) {
    char ch = buf[i];
    switch (state->mode) {
      case REPL_MODE_NORMAL:
        handle_normal(ch);
        break;
      case REPL_MODE_ESCAPE:
        handle_escape(ch);
        break;
    }
  }
}
#if 0 //Never used.
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
#endif
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
 * .reset command
 */
static void cmd_reset(repl_state_t *state) {
  runtime_cleanup();
  runtime_init(false);
}

static size_t bytes_remained = 0;

static int header_cb(uint8_t *file_name, size_t file_size) {
  flash_program_begin();
  bytes_remained = file_size;
  return 0;
}

static int packet_cb(uint8_t *data, size_t len) {
  if (bytes_remained < len) {
    len = bytes_remained;
    bytes_remained = 0;
  } else {
    bytes_remained = bytes_remained - len;
  }
  flash_status_t status = flash_program(data, len);
  if (status == FLASH_SUCCESS) {
    return 0;
  } else {
    return -1;
  }
  return 0;
}

static void footer_cb() {
  flash_program_end();
  bytes_remained = 0;
}

/**
 * .flash command
 */
static void cmd_flash(repl_state_t *state, char *arg) {
  /* erase flash */
  if (strcmp(arg, "-e") == 0) {
    flash_clear();
    repl_printf("Flash has erased\r\n");

  /* get total size of flash */
  } else if (strcmp(arg, "-t") == 0) {
    uint32_t size = flash_size();
    repl_printf("%u\r\n", size);

  /* get data size in flash */
  } else if (strcmp(arg, "-s") == 0) {
    uint32_t data_size = flash_get_data_size();
    repl_printf("%u\r\n", data_size);

  /* read data from flash */
  } else if (strcmp(arg, "-r") == 0) {
    uint32_t sz = flash_get_data_size();
    uint8_t *ptr = flash_get_data();
    for (int i = 0; i < sz; i++) {
      if (ptr[i] == '\n') { /* convert "\n" to "\r\n" */
        repl_putc('\r');
      }
      repl_putc(ptr[i]);
    }
    repl_println();

  /* write a file to flash via Ymodem */
  } else if (strcmp(arg, "-w") == 0) {
    state->ymodem_state = 1; // transfering
    tty_printf("Transfer a file via Ymodem... (press 'a' to abort)\r\n");
    io_tty_read_stop(&tty);
    ymodem_status_t result = ymodem_receive(header_cb, packet_cb, footer_cb);
    io_tty_read_start(&tty, tty_read_cb);
    delay(500);
    switch (result) {
      case YMODEM_OK:
        tty_printf("\r\nDone.\r\n");
        break;
      case YMODEM_LIMIT:
        tty_printf("\r\nThe file size is too large.\r\n");
        break;
      case YMODEM_DATA:
        tty_printf("\r\nVerification failed.\r\n");
        break;
      case YMODEM_ABORT:
        tty_printf("\r\nAborted.\r\n");
        break;
      default:
        tty_printf("\r\nFailed to receive.\r\n");
        break;
    }
    state->ymodem_state = 0; // stopped
  /* no option is given */
  } else {
    repl_printf(".flash command options:\r\n");
    repl_printf("-w\tWrite user code (file) to flash via Ymodem.\r\n");
    repl_printf("-e\tErase the user code in flash.\r\n");
    repl_printf("-t\tPrint total size of flash for user code.\r\n");
    repl_printf("-s\tPrint the size of the user code.\r\n");
    repl_printf("-r\tPrint the user code in textual format.\r\n");
  }
}

/**
 * .load command
 */
static void cmd_load(repl_state_t *state) {
  runtime_cleanup();
  runtime_init(true);
}

/**
 * .mem command
 */
static void cmd_mem(repl_state_t *state) {
  jerry_heap_stats_t stats = {0};
  bool stats_ret = jerry_get_memory_stats (&stats);
  if (stats_ret) {
    repl_printf("total: %u, occupied: %u, peak: %u\r\n", stats.size, stats.allocated_bytes, stats.peak_allocated_bytes);
  } else {
    repl_printf("Mem stat feature is not enabled.\r\n");
  }
}

/**
 * .gc command
 */
static void cmd_gc(repl_state_t *state) {
  jerry_gc(JERRY_GC_PRESSURE_HIGH);
}

/**
 * .firmup command
 */
static void cmd_firmup(repl_state_t *state) {
  request_firmup();
}

/**
 * .hi command
 */
static void cmd_hi(repl_state_t *state) {
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
  repl_printf("For more info: https://kameleon.io\r\n");
  repl_println();
}

/**
 * .help command
 */
static void cmd_help(repl_state_t *state) {
  repl_printf(".echo\tEcho on/off.\r\n");
  repl_printf(".reset\tReset JavaScript runtime context.\r\n");
  repl_printf(".flash\tCommands for the internal flash.\r\n");
  repl_printf(".load\tLoad user code from the internal flash.\r\n");
  repl_printf(".mem\tHeap memory status.\r\n");
  repl_printf(".firmup\tFirmware update mode.\r\n");
  repl_printf(".gc\tPerform garbage collection.\r\n");
  repl_printf(".hi\tPrint welcome message.\r\n");
  repl_printf(".help\tPrint this help message.\r\n");
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
  state.width = 80;
  state.escape_length = 0;
  state.history_size = 0;
  state.history_position = 0;
  state.handler = &default_handler;
  state.ymodem_state = 0;
  cmd_hi(NULL);
}

repl_state_t *get_repl_state() {
  return &state;
}

void repl_set_output(repl_output_t output) {
  switch (output) {
    case REPL_OUTPUT_NORMAL:
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

void repl_println() {
  tty_printf("\r\n");
}

void repl_print_prompt() {
  tty_printf("\33[0m"); // back to normal color
  if (state.echo) {
    state.buffer[state.buffer_length] = '\0';
    tty_printf("\r\033[s"); // save cursor position
    tty_printf("> %s", &state.buffer);
    tty_printf("\33[H\33[900C\33[6n\033[u\033[2C"); // query terminal screen width and restore cursor position
  }
}
