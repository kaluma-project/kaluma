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
#include "adc.h"
#include "kameleon_core.h"
#include <driver/adc.h>
#include <driver/gpio.h>
#include <esp_adc_cal.h>
#include <soc/adc_channel.h>

#define DEFAULT_VREF 1100
#define NO_OF_SAMPLES 64

static esp_adc_cal_characteristics_t *adc_chars = NULL;
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unit = ADC_UNIT_1;

void adc_init()
{
		
}

void adc_cleanup()
{
}

static uint8_t pin_to_channel(uint8_t pin)
{
	switch(pin)
	{
		case 32: return ADC1_GPIO32_CHANNEL;
		case 33: return ADC1_GPIO33_CHANNEL;
		case 34: return ADC1_GPIO34_CHANNEL;
		case 35: return ADC1_GPIO35_CHANNEL;
		case 36: return ADC1_GPIO36_CHANNEL;
		case 37: return ADC1_GPIO37_CHANNEL;
		case 38: return ADC1_GPIO38_CHANNEL;
		case 39: return ADC1_GPIO39_CHANNEL;
		default: return 0;
	}
}

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

int adc_setup(uint8_t pin)
{
	uint8_t channel = pin_to_channel(pin);
	if ( channel == 0 ) return -1;

	if ( adc_chars ) return channel;
	adc1_config_width(width);
	adc1_config_channel_atten(channel, atten);
	adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
	esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width, DEFAULT_VREF, adc_chars);
	print_char_val_type(val_type);
	return channel;
}

double adc_read(uint8_t adcIndex)
{
	uint32_t reading = adc1_get_raw(adcIndex);
	uint32_t voltage = esp_adc_cal_raw_to_voltage(reading, adc_chars);
	//printf("adc_read(%d) : %d, %d\n", adcIndex, reading, voltage);
	return voltage / 1000.0;
}

int adc_close(uint8_t pin)
{
	if ( adc_chars == NULL ) return 0;
	free(adc_chars);
	adc_chars = NULL;
	return 0;
}

