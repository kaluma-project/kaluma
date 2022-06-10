/* Copyright (c) 2017 Pico
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
#include <math.h>

#include "err.h"
#include "math.h"
#include "hardware/structs/systick.h"
#include "hardware/structs/sio.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"
#include "jerryscript.h"
#include "jerryxx.h"
#include "pwm.h"
#include "pwmaudio_magic_strings.h"

#include "pico/time.h"

#define PWM_AUDIO_PIN (6)

queue_t note_queue; /* awww aint u a queue_t */
typedef struct { uint16_t freq, ms; } note_t;

void audio_main(void) {
  while (true) {
    note_t note;
    queue_remove_blocking(&note_queue, &note);
    km_pwm_set_frequency(PWM_AUDIO_PIN, note.freq);

    /* play a note at that frequency */
    km_pwm_set_duty(PWM_AUDIO_PIN, 0.5); {
      sleep_until(make_timeout_time_ms(note.ms));
    } km_pwm_set_duty(PWM_AUDIO_PIN, 0);
  }
}

JERRYXX_FUN(pwmaudio_start_ticker_fn) {
  static uint8_t started = 0;

  if (!started) {
    started = 1;

    km_pwm_setup(PWM_AUDIO_PIN, 220, 0);
    km_pwm_start(PWM_AUDIO_PIN);

    queue_init(&note_queue, sizeof(note_t), 1 << 5);
    multicore_launch_core1(audio_main);

    return jerry_create_boolean(true);
  }
  return jerry_create_boolean(false);
}

JERRYXX_FUN(pwmaudio_play_note) {
  JERRYXX_CHECK_ARG_NUMBER(0, "frequency");
  JERRYXX_CHECK_ARG_NUMBER(1, "milliseconds");

  return jerry_create_boolean(queue_try_add(&note_queue, &(note_t){
    .freq = (uint16_t)JERRYXX_GET_ARG_NUMBER(0),
    .ms   = (uint16_t)JERRYXX_GET_ARG_NUMBER(1),
  }));
}

jerry_value_t module_pwmaudio_init() {
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property_function(exports, MSTR_PWMAUDIO_START_TICKER,
                                pwmaudio_start_ticker_fn);
  jerryxx_set_property_function(exports, MSTR_PWMAUDIO_PLAY_NOTE,
                                pwmaudio_play_note);

  return exports;
}
