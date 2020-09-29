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

#include <stdint.h>
#include <freertos/FreeRTOS.h>
#include <driver/ledc.h>
#include <driver/gpio.h>
#include "pwm.h"
#include "gpio.h"
#include "esp32_devkitc.h"

#define MIN_FREQ        1
#define MAX_FREQ        300000
static ledc_timer_config_t timer_config = {
  .duty_resolution = LEDC_TIMER_13_BIT,
  .freq_hz = 490,
  .speed_mode = LEDC_HIGH_SPEED_MODE,
  .timer_num = LEDC_TIMER_0,
  .clk_cfg = LEDC_AUTO_CLK,
};

static ledc_channel_config_t channel = {
  .channel = LEDC_CHANNEL_0,
  .duty = 0,
  .gpio_num = 0,
  .speed_mode = LEDC_HIGH_SPEED_MODE,
  .hpoint = 0,
  .timer_sel = LEDC_TIMER_0,
};

void pwm_init()
{
}

void pwm_cleanup()
{
}

static int8_t set_pwm_timer(uint8_t pin)
{
  switch (pin)
  {
    case 2:
      timer_config.timer_num = LEDC_TIMER_0;
      channel.timer_sel = LEDC_TIMER_0;
      channel.channel = LEDC_CHANNEL_0;
      channel.gpio_num = GPIO_NUM_21;
      break;
    case 12:
      timer_config.timer_num = LEDC_TIMER_1;
      channel.timer_sel = LEDC_TIMER_1;
      channel.channel = LEDC_CHANNEL_1;
      channel.gpio_num = GPIO_NUM_13;
      break;
    case 18:
      timer_config.timer_num = LEDC_TIMER_2;
      channel.timer_sel = LEDC_TIMER_2;
      channel.channel = LEDC_CHANNEL_2;
      channel.gpio_num = GPIO_NUM_33;
      break;
    case 19:
      timer_config.timer_num = LEDC_TIMER_3;
      channel.timer_sel = LEDC_TIMER_3;
      channel.channel = LEDC_CHANNEL_3;
      channel.gpio_num = GPIO_NUM_32;
      break;
    default:
      return -1; // Error
  }
  return 0;
}

static int8_t get_pwm_channel(uint8_t pin)
{
  switch (pin)
  {
    case 2:
      return LEDC_CHANNEL_0;
    case 12:
      return LEDC_CHANNEL_1;
    case 18:
      return LEDC_CHANNEL_2;
    case 19:
      return LEDC_CHANNEL_3;
  }
  return -1; // Error
}

static int8_t get_resolution(uint32_t freq)
{
  if (freq <= 8000) {
    return LEDC_TIMER_13_BIT;
  } else if (freq <= 70000) {
    return LEDC_TIMER_10_BIT;
  } else if (freq <= 150000) {
    return LEDC_TIMER_9_BIT;
  }
  return LEDC_TIMER_8_BIT;
}

static uint32_t get_duby_div(uint32_t duty_res)
{
  return (1 << duty_res);
}
int pwm_setup(uint8_t pin, double frequency, double duty)
{
  if (set_pwm_timer(pin) == -1) {
    return -1;
  }
  if (frequency < MIN_FREQ) {
    frequency = MIN_FREQ;
  } else if (frequency > MAX_FREQ) {
    frequency = MAX_FREQ;
  }
  printf("pwm_setup(%d, %lf, %lf)\n", pin ,frequency, duty);
  int8_t duty_res = get_resolution(frequency);
  timer_config.freq_hz = (uint32_t)frequency;
  timer_config.duty_resolution = duty_res;
  channel.duty = (uint32_t)(duty * get_duby_div(duty_res));

  ledc_timer_config(&timer_config);
  ledc_channel_config(&channel);
  ledc_stop(LEDC_HIGH_SPEED_MODE, channel.channel, 0);
  return 0;
}

int pwm_start(uint8_t pin)
{
  printf("pwm_start(%d)\n", pin);
  int8_t channel = get_pwm_channel(pin);
  if (channel == -1) {
    return -1;
  }
  ledc_update_duty(LEDC_HIGH_SPEED_MODE, channel);
  return 0;
}

int pwm_stop(uint8_t pin)
{
  printf("pwm_stop(%d)\n", pin);
  int8_t channel = get_pwm_channel(pin);
  if (channel == -1) {
    return -1;
  }
  ledc_stop(LEDC_HIGH_SPEED_MODE, channel, 0);
  return 0;
}

double pwm_get_frequency(uint8_t pin)
{
  int8_t channel = get_pwm_channel(pin);
  if (channel == -1) {
    return -1;
  }
  return (double)ledc_get_freq(LEDC_HIGH_SPEED_MODE, channel);
}

int pwm_set_frequency(uint8_t pin, double frequency)
{
  if (set_pwm_timer(pin) == -1) {
    return -1;
  }
  if (frequency < MIN_FREQ) {
    frequency = MIN_FREQ;
  } else if (frequency > MAX_FREQ) {
    frequency = MAX_FREQ;
  }
  int8_t duty_res = get_resolution(frequency);
  timer_config.freq_hz = frequency;
  timer_config.duty_resolution = duty_res;
  ledc_timer_config(&timer_config);
  return 0;
}

double pwm_get_duty(uint8_t pin)
{
  int8_t channel = get_pwm_channel(pin);
  if (channel == -1) {
    return -1;
  }
  int duty_res = get_resolution(pwm_get_frequency(pin));
  return (double)ledc_get_duty(LEDC_HIGH_SPEED_MODE, channel) / get_duby_div(duty_res);
}

int pwm_set_duty(uint8_t pin, double duty)
{
  if (set_pwm_timer(pin) == -1) {
    return -1;
  }
  int duty_res = get_resolution(pwm_get_frequency(pin));
  channel.duty = (uint32_t)(duty * get_duby_div(duty_res));
  ledc_channel_config(&channel);
  return 0;
}

int pwm_close(uint8_t pin)
{
  if (set_pwm_timer(pin) == -1) {
    return -1;
  }
  ledc_stop(LEDC_HIGH_SPEED_MODE, channel.channel, 0);
  gpio_set_io_mode(pin, GPIO_IO_MODE_INPUT);
  return 0;
}

