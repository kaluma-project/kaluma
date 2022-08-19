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
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include "jerryscript.h"
#include "jerryxx.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "pico/multicore.h"
#include "i2saudio_magic_strings.h"

#define I2S_CLK_PINS 10
#define I2S_DATA_PIN 9

typedef enum {
  Sound_Sine,
  Sound_Square,
  Sound_Triangle,
  Sound_Sawtooth,
  Sound_COUNT,
} Sound;

static uint32_t freq_to_delta(float freq) { return freq/16000.0*256.0*65536.0; }

uint32_t ticks = 0;

typedef struct {
  uint32_t phase, delta;
  uint16_t volume;
  Sound sound;
} Channel;

#define FIFO_LENGTH (1 << 8)
queue_t message_fifo;
typedef enum {
  MessageKind_PushFreq,
  MessageKind_Wait,
} MessageKind;
typedef struct {
  MessageKind kind;
  unsigned int duration; /* for MessageKind_Wait */
  float freq;  /* for MessageKind_PushFreq */
  Sound sound; /* for MessageKind_PushFreq */
} Message;

#define CHANNEL_COUNT (32)
static struct {
  Channel channels[CHANNEL_COUNT];
} state;

#define SOUND_SAMPLE_LEN (1 << 9)
typedef struct { int data[SOUND_SAMPLE_LEN]; } SoundSample;
static SoundSample sound_samples[Sound_COUNT];
#define MAX_PHASE (0xffffff)

void pio0_irq_0_handler(void) {
  uint32_t ints = pio0->ints0;
  uint16_t sample = 0x1000;

  ints >>= 8;
  pio0->irq = ints;

  /* square wave */
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    Channel *chan = state.channels + i;

    if (chan->volume)
      sample += sound_samples[chan->sound].data[chan->phase / (MAX_PHASE/SOUND_SAMPLE_LEN)];
  }

  pio_sm_put(pio0, 0, sample<<16);

  for (int i = 0; i < CHANNEL_COUNT; i++) {
    Channel *chan = state.channels + i;

    chan->phase += chan->delta;
    chan->phase &= 0xffffff;
  }
  
  ticks++;
}

/*
.wrap_target
  set x 14      side 0b01
left:
  out pins 1    side 0b00
  jmp x-- left  side 0b01
  out pins 1    side 0b10  
  set x 14      side 0b11
right:
  out pins 1    side 0b10  
  jmp x-- right side 0b11
  out pins 1    side 0b00  
.wrap
*/

uint16_t pio_instructions_write_16[] = {0xe82e, 0x6001, 0x0841, 0x7001, 0xf82e, 0x7001, 0x1845, 0x6001};

pio_program_t pio_write_16 = {
    pio_instructions_write_16,
    sizeof(pio_instructions_write_16) / sizeof(uint16_t),
    -1
};

// JERRYXX_FUN(i2saudio_ticks_fn) { return jerry_create_number(ticks); }

JERRYXX_FUN(i2saudio_wait_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "time");
  uint16_t v = jerry_value_as_uint32(JERRYXX_GET_ARG(0));

  Message msg = { .kind = MessageKind_Wait, .duration = v };
  if (!queue_try_add(&message_fifo, &msg))
    printf("FIFO was full\n");

  return jerry_create_undefined();
}

JERRYXX_FUN(i2saudio_push_freq_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "freq");
  uint16_t v = jerry_value_as_uint32(JERRYXX_GET_ARG(0));
  Sound sound = JERRYXX_GET_ARG_NUMBER(1);

  Message msg = { .kind = MessageKind_PushFreq, .sound = sound, .freq = v };
  if (!queue_try_add(&message_fifo, &msg))
    printf("FIFO was full\n");

  return jerry_create_undefined();
}

// JERRYXX_FUN(i2saudio_setvolume1_fn) {
//   JERRYXX_CHECK_ARG_NUMBER(0, "setvolume1");
//   volume1 = (uint16_t)jerry_value_as_uint32(JERRYXX_GET_ARG(0));
//   return jerry_create_undefined();
// }
// 
// JERRYXX_FUN(i2saudio_setfreq1_fn) {
//   JERRYXX_CHECK_ARG_NUMBER(0, "setfreq1");
//   double freq = jerry_get_number_value(JERRYXX_GET_ARG(0));
//   double ddelta = freq/16000.0*256.0*65536.0;
//   delta1 = (uint32_t) ddelta;
//   return jerry_create_undefined();
// }

JERRYXX_FUN(i2saudio_midi_fn) {
  JERRYXX_CHECK_ARG_NUMBER(0, "midi");
  int32_t n = jerry_value_as_int32(JERRYXX_GET_ARG(0));
  double freq = 130.8128*pow(1.0594630943592953,n-48);
  return jerry_create_number(freq);
}

void core1_entry(void) {
  pio_sm_set_enabled(pio0, 0, false);
  pio_gpio_init(pio0, I2S_CLK_PINS);
  pio_gpio_init(pio0, I2S_CLK_PINS+1);
  pio_gpio_init(pio0, I2S_DATA_PIN);
  pio_add_program_at_offset(pio0, &pio_write_16,0);
  pio_sm_config c = pio_get_default_sm_config();
  sm_config_set_out_pins(&c, I2S_DATA_PIN, 1);
  sm_config_set_sideset_pins(&c, I2S_CLK_PINS);
  sm_config_set_sideset(&c, 2, false, false);
  sm_config_set_wrap(&c, 0, pio_write_16.length-1);
  sm_config_set_out_shift(&c, false, true, 32); 
  sm_config_set_clkdiv(&c, 122.07);  // 125000000 / 16000 / 32 / 2 
  pio_sm_set_consecutive_pindirs(pio0, 0, I2S_CLK_PINS, 2, true);
  pio_sm_set_consecutive_pindirs(pio0, 0, I2S_DATA_PIN, 1, true);
  pio_sm_init(pio0, 0, 0, &c);
  pio_sm_set_enabled(pio0, 0, true);

  irq_set_exclusive_handler(PIO0_IRQ_0, pio0_irq_0_handler);
  irq_set_enabled(PIO0_IRQ_0, true);

  pio0->inte0 |= PIO_INTR_SM0_TXNFULL_BITS;

  for (int i = 0; i < SOUND_SAMPLE_LEN; i++) {
    float t = (float)i / (float)SOUND_SAMPLE_LEN;
    sound_samples[Sound_Sine    ].data[i] = sinf(t * M_PI * 2) * 7500;
    sound_samples[Sound_Square  ].data[i] = (t > 0.5f) ? 7500 : 0;
    sound_samples[Sound_Triangle].data[i] = fabsf(0.5f - fmodf(t, 1.0f)) * 7500;
    sound_samples[Sound_Sawtooth].data[i] = t * 7500;
  }

  while (true) {
    int i = 0;
    int duration = 0;

    Message message;
    while(true) {
      queue_remove_blocking(&message_fifo, &message);

      if (message.kind == MessageKind_PushFreq) {
        switch (message.sound) {
        case Sound_Sine    : puts("Sound_Sine    "); break;
        case Sound_Square  : puts("Sound_Square  "); break;
        case Sound_Triangle: puts("Sound_Triangle"); break;
        case Sound_Sawtooth: puts("Sound_Sawtooth"); break;
        default: puts("Sound_Wtf"); break;
        }

        Channel *chan = state.channels + i++;
        chan->volume = 1500;
        chan->delta = freq_to_delta(message.freq);
        chan->sound = message.sound;
      } else if (message.kind == MessageKind_Wait) {
        duration = message.duration;
        break;
      }
    };

    sleep_ms(duration);

    /* turn 'em off and wait for more data */
    for (int i = 0; i < CHANNEL_COUNT; i++)
      state.channels[i].volume = 0;
  }
}

jerry_value_t module_i2saudio_init(void) {

  static int core_needs_init = 1;
  if (core_needs_init) {
    core_needs_init = 0;
    queue_init(&message_fifo, sizeof(Message), FIFO_LENGTH);
    multicore_launch_core1(core1_entry);
  }

  jerry_value_t exports = jerry_create_object();
  // jerryxx_set_property_function(exports, MSTR_I2SAUDIO_TICKS, i2saudio_ticks_fn);

  jerryxx_set_property_function(exports, MSTR_I2SAUDIO_WAIT, i2saudio_wait_fn);
  jerryxx_set_property_function(exports, MSTR_I2SAUDIO_PUSH_FREQ, i2saudio_push_freq_fn);

  jerryxx_set_property_function(exports, MSTR_I2SAUDIO_MIDI, i2saudio_midi_fn);
  return exports;
}
