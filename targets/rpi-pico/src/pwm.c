/* Copyright (c) 2017 Kaluma
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
#include "pwm.h"
#include "rpi_pico.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

static struct __pwm_config_s {
  double freq;
  double duty;
  uint16_t period;
  bool enabled;
} __pwm_config[PWM_NUM];

static int __get_pwm_index(uint8_t pin)
{
  if (((pin % 2) == 0) && (pin >= 8) && (pin <= 22)) {
    // 8, 10, 12, 14, 16, 18, 20, 22
    return pwm_gpio_to_slice_num(pin);
  }
  return KM_PWMPORT_ERROR;
}

/**
 * Initialize all PWM when system started
 */
void km_pwm_init() {
  for (int i = 0; i < PWM_NUM; i++)
  {
    __pwm_config[i].freq = 0;
    __pwm_config[i].duty = 0;
    __pwm_config[i].period = 0;
    __pwm_config[i].enabled = false;
  }
}

/**
 * Cleanup all PWM when system cleanup
 */
void km_pwm_cleanup() {
  km_pwm_init();
}

uint16_t __get_period(double frequency)
{
  int ref_div = 1;
  int duty_mul = 1;
  if (frequency > 100000) {
    ref_div = 10;
    if (frequency > 1000000) {
      frequency = 1000000;
    }
  } else if (frequency < 25) {
    duty_mul = 32;
  } else if (frequency < 50) {
    duty_mul = 16;
  } else if (frequency < 100) {
    duty_mul = 8;
  } else if (frequency < 200) {
    duty_mul = 4;
  } else if (frequency < 400) {
    duty_mul = 2;
  };
  return (PWM_CLK_REF / ref_div) * duty_mul;
}
/**
 * return Returns 0 on success or -1 on failure.
*/
int km_pwm_setup(uint8_t pin, double frequency, double duty) {
  int pwm_index = __get_pwm_index(pin);
  if (pwm_index < 0) {
    return KM_PWMPORT_ERROR; // Error
  }
  if (frequency < 13) {
    frequency = 13; // Min is 13Hz
  }
  uint16_t period = __get_period(frequency);
  double clk_div = clock_get_hz(clk_sys) / (frequency * period);
  gpio_set_function(pin, GPIO_FUNC_PWM);
  pwm_config config = pwm_get_default_config();
  pwm_config_set_clkdiv(&config, clk_div);
  pwm_config_set_wrap(&config, period - 1);
  if (__pwm_config[pwm_index].enabled) {
    pwm_init(pwm_index, &config, true);
  } else {
    pwm_init(pwm_index, &config, false);
  }
  uint16_t uint_duty = (uint16_t)(duty * period);
  pwm_set_chan_level(pwm_index, pwm_gpio_to_channel(pin), uint_duty);
  __pwm_config[pwm_index].freq = frequency;
  __pwm_config[pwm_index].duty = duty;
  __pwm_config[pwm_index].period = period;
  return 0;
}

/**
*/
int km_pwm_start(uint8_t pin) {
  int pwm_index = __get_pwm_index(pin);
  if (pwm_index < 0) {
    return KM_PWMPORT_ERROR; // Error
  }
  pwm_set_enabled(pwm_index, true);
  __pwm_config[pwm_index].enabled = true;
  return 0;
}

/**
*/
int km_pwm_stop(uint8_t pin) {
  int pwm_index = __get_pwm_index(pin);
  if (pwm_index < 0) {
    return KM_PWMPORT_ERROR; // Error
  }
  pwm_set_enabled(pwm_index, false);
  __pwm_config[pwm_index].enabled = false;
  return 0;
}

/**
*/
double km_pwm_get_frequency(uint8_t pin) {
  int pwm_index = __get_pwm_index(pin);
  if (pwm_index < 0) {
    return KM_PWMPORT_ERROR; // Error
  }
  return __pwm_config[pwm_index].freq;
}

/**
*/
double km_pwm_get_duty(uint8_t pin) {
  int pwm_index = __get_pwm_index(pin);
  if (pwm_index < 0) {
    return KM_PWMPORT_ERROR; // Error
  }
  return __pwm_config[pwm_index].duty;
}

/**
*/
int km_pwm_set_duty(uint8_t pin, double duty) {
  int pwm_index = __get_pwm_index(pin);
  if (pwm_index < 0) {
    return KM_PWMPORT_ERROR; // Error
  }
  uint16_t uint_duty = (uint16_t)(duty * __pwm_config[pwm_index].period);
  if (__pwm_config[pwm_index].enabled) {
    while (pwm_get_counter(pwm_index) != 0);
  }
  pwm_set_chan_level(pwm_index, pwm_gpio_to_channel(pin), uint_duty);
  __pwm_config[pwm_index].duty = duty;
  return 0;
}

/**
*/
int km_pwm_set_frequency(uint8_t pin, double frequency) {
  int pwm_index = __get_pwm_index(pin);
  if (pwm_index < 0) {
    return KM_PWMPORT_ERROR; // Error
  }
  double previous_duty = km_pwm_get_duty(pin);
  /* The previous duty ratio must be hold up regardless of changing frequency */
  if (__pwm_config[pwm_index].enabled) {
    while (pwm_get_counter(pwm_index) != 0);
  }
  km_pwm_setup(pin, frequency, previous_duty);
  return 0;
}

/**
*/
int km_pwm_close(uint8_t pin) {
  int pwm_index = __get_pwm_index(pin);
  if (pwm_index < 0) {
    return KM_PWMPORT_ERROR; // Error
  }
  if (__pwm_config[pwm_index].enabled) {
    km_pwm_stop(pin);
  }
  return 0;
}
