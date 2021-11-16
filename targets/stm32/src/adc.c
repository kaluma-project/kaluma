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

#include "adc.h"

#include <stdint.h>

#include "board.h"

DMA_HandleTypeDef hdma_adc1;
static uint16_t adc_buf[ADC_NUM];
static uint8_t adc_configured[ADC_NUM];
static ADC_HandleTypeDef hadc1;

static const struct __adc_config {
  uint8_t pin_number;
  GPIO_TypeDef* port;
  uint32_t pin;
  uint32_t channel;
} adc_config[] = {
    // Same as ADC_NUM
    {3, GPIOA, GPIO_PIN_1, ADC_CHANNEL_1},
    {4, GPIOA, GPIO_PIN_2, ADC_CHANNEL_2},
    {5, GPIOA, GPIO_PIN_3, ADC_CHANNEL_3},
    {10, GPIOA, GPIO_PIN_5, ADC_CHANNEL_5},
    {11, GPIOA, GPIO_PIN_6, ADC_CHANNEL_6},
    {12, GPIOA, GPIO_PIN_7, ADC_CHANNEL_7},
};

/**
 * Get ADC index
 *
 * @param pin Pin number.
 * @return Returns index on success or -1 on failure.
 */
/**
 * input : pinNumber
 * output : pinIndex or ADC_PORTERRROR (-1)
 *          0xFF means the pin is not assigned for ADC
 */
static int get_adc_index(uint8_t pin) {
  uint32_t n = sizeof(adc_config) / sizeof(struct __adc_config);
  int index = KM_ADCPORT_ERRROR;

  for (int k = 0; k < n; k++) {
    if (adc_config[k].pin_number == pin) {
      index = k;
      break;
    }
  }
  return index;
}

/* ADC1 init function */
void adc1_init() {
  uint32_t n = sizeof(adc_config) / sizeof(struct __adc_config);
  ADC_ChannelConfTypeDef sConfig;

  /**Configure the global features of the ADC (Clock, Resolution, Data Alignment
   * and number of conversion)
   */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = ADC_NUM;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) {
    _Error_Handler(__FILE__, __LINE__);
  }

  /**Configure for the selected ADC regular channel its corresponding rank in
   * the sequencer and its sample time.
   */

  for (int k = 0; k < n; k++) {
    sConfig.Channel = adc_config[k].channel;
    sConfig.Rank = k + 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
      _Error_Handler(__FILE__, __LINE__);
    }
  }
}

/**
 */
static void adc1_deinit() { HAL_ADC_DeInit(&hadc1); }

/**
 * Initialize all ADC channels when system started
 */
void km_adc_init() {}

/**
 * Cleanup all ADC channels when system cleanup
 */
void km_adc_cleanup() {
  uint32_t n = sizeof(adc_config) / sizeof(struct __adc_config);
  for (int k = 0; k < n; k++) {
    if (adc_configured[k]) km_adc_close(adc_config[k].pin_number);
  }
}

/**
 * Read value from the ADC channel
 *
 * @param {uint8_t} adcIndex
 * @return {double}
 */
double km_adc_read(uint8_t adcIndex) {
  HAL_ADC_Start(&hadc1);

  for (int k = 0; k < ADC_NUM; k++) {
    HAL_StatusTypeDef status = HAL_ADC_PollForConversion(&hadc1, 100);

    if (status == HAL_TIMEOUT) {
      while (1)
        ;
    }

    adc_buf[k] = HAL_ADC_GetValue(&hadc1);
  }

  HAL_ADC_Stop(&hadc1);

  return (double)adc_buf[adcIndex] / (1 << ADC_RESOLUTION_BIT);
}

int km_adc_setup(uint8_t pin) {
  int n = get_adc_index(pin);
  if (n == KM_ADCPORT_ERRROR) return KM_ADCPORT_ERRROR;

  uint8_t adc_need_init = 1;
  for (int k = 0; k < ADC_NUM; k++) {
    if (adc_configured[k]) {
      adc_need_init = 0;
      break;
    }
  }
  if (adc_need_init) {
    adc1_init();
  }

  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = adc_config[n].pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(adc_config[n].port, &GPIO_InitStruct);

  adc_configured[n] = 1;
  return n;
}

int km_adc_close(uint8_t pin) {
  int n = get_adc_index(pin);
  if (n == KM_ADCPORT_ERRROR) return KM_ADCPORT_ERRROR;
  HAL_GPIO_DeInit(adc_config[n].port, adc_config[n].pin);
  adc_configured[n] = 0;

  uint8_t adc_need_deinit = 1;
  for (int k = 0; k < ADC_NUM; k++) {
    if (adc_configured[k]) {
      adc_need_deinit = 0;
      break;
    }
  }
  if (adc_need_deinit) {
    adc1_deinit();
  }
  return n;
}
