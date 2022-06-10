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
#include "jerryscript.h"
#include "jerryxx.h"
#include "pwm.h"
#include "pwmaudio_magic_strings.h"

#include "pico/time.h"

#define PWM_AUDIO_PIN (6)

#define NOTE_TICKS (0.5)
float notes[] = {
  440, 294, 587, 784,
  880, 784, 699, 659,
  523, 440, 349, 330,
  330, 330, 349,   0,
  784, 699, 659, 349,
    0, 350, 349, 349,
  392, 659, 659, 587,
  587, 523, 523, 494,
};
#define NNOTE (sizeof(notes) / sizeof(notes[0]))

void audio_main(void) {
  int i = 0;
  while (true) {
    i = (i + 1) % NNOTE;
    float note = notes[i];
    // float f = powf(2, (note-69)/12)*440;
    km_pwm_set_frequency(PWM_AUDIO_PIN, note);
    sleep_until(make_timeout_time_ms(0.5 * 1000));
  }
}

JERRYXX_FUN(pwmaudio_start_ticker_fn) {
  km_pwm_setup(PWM_AUDIO_PIN, 220, 0.5);
  km_pwm_start(PWM_AUDIO_PIN);

  multicore_launch_core1(audio_main);

  return jerry_create_undefined();
}

jerry_value_t module_pwmaudio_init() {
  jerry_value_t exports = jerry_create_object();
  jerryxx_set_property_function(exports, MSTR_PWMAUDIO_START_TICKER,
                                pwmaudio_start_ticker_fn);

  return exports;
}
