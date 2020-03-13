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
#include "pwm.h"
#include "kameleon_core.h"


static ledc_timer_config_t timer_config = {
	.duty_resolution = LEDC_TIMER_13_BIT,
	.freq_hz = 5000,
	.speed_mode = LEDC_HIGH_SPEED_MODE,
	.timer_num = LEDC_TIMER_0,
	.clk_cfg = LEDC_AUTO_CLK
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

int pwm_setup(uint8_t pin, double frequency, double duty)
{
	printf("pwm_setup(%d, %lf, %lf)\n", pin ,frequency, duty);
	timer_config.freq_hz = (uint32_t)frequency;
	channel.gpio_num = pin;
	channel.duty = (uint32_t)(8192*duty);

	ledc_timer_config(&timer_config);
	ledc_channel_config(&channel);
	return 0;
}

int pwm_start(uint8_t pin)
{
	printf("pwm_start(%d)\n", pin);
	if (channel.gpio_num != pin ) 
	{
		printf("pwm_start return -1\n");
		return -1;
	}
	ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
	return 0;
}

int pwm_stop(uint8_t pin)
{
	printf("pwm_stop(%d)\n", pin);
	if (channel.gpio_num != pin ) return -1;
	ledc_stop(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0);
	return 0;
}

double pwm_get_frequency(uint8_t pin)
{
	if (channel.gpio_num != pin ) return -1;
	return timer_config.freq_hz;
}

int pwm_set_frequency(uint8_t pin, double frequency)
{
	if (channel.gpio_num != pin ) return -1;
	timer_config.freq_hz = frequency;
	ledc_timer_config(&timer_config);
	return 0;
}

double pwm_get_duty(uint8_t pin)
{
	if (channel.gpio_num != pin ) return 0.;
	return (double)channel.duty/8192;
}

int pwm_set_duty(uint8_t pin, double duty)
{
	if (channel.gpio_num != pin ) return -1;
	channel.duty = (uint32_t)(duty*8192);
	ledc_channel_config(&channel);
	return 0;
}

int pwm_close(uint8_t pin)
{
  return 0;
}

