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
#include "stm32f4discovery.h"

DMA_HandleTypeDef hdma_adc1;
static uint16_t adc_buf[ADC_NUM];
static uint8_t adc_configured[ADC_NUM];
static ADC_HandleTypeDef hadc1;

static const struct __adc_config {
    uint8_t pin_number;
    GPIO_TypeDef * port;
    uint32_t pin;
    uint32_t channel;
} adc_config[] = {
   {23, GPIOC, GPIO_PIN_1, ADC_CHANNEL_11},
   {25, GPIOA, GPIO_PIN_1, ADC_CHANNEL_1},
   {28, GPIOC, GPIO_PIN_5, ADC_CHANNEL_15},
   {29, GPIOC, GPIO_PIN_4, ADC_CHANNEL_14},
   {30, GPIOB, GPIO_PIN_1, ADC_CHANNEL_9},
   {31, GPIOB, GPIO_PIN_0, ADC_CHANNEL_8},
   {69, GPIOC, GPIO_PIN_3, ADC_CHANNEL_13},
   {70, GPIOA, GPIO_PIN_0, ADC_CHANNEL_0},
};


/**
*/
static uint8_t get_adc_index(uint8_t pin) {
  uint32_t n = sizeof(adc_config) / sizeof(struct __adc_config);
  uint8_t index;

  for (int k=0; k<n; k++) {
    if (adc_config[k].pin_number == pin) {
      index = k;
      break;
    }
  }  
  
  return index;    
}

/** 
*/
static void adc1_start_dma() {
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buf, ADC_NUM);
}

/** 
*/
static uint8_t adc_need_init() {
  uint32_t sum=0;
  for (uint32_t k=0; k<ADC_NUM; k++) {
    sum = sum + adc_configured[k];
  }  
  return (sum==0) ? 1:0;
}

/** 
*/
static uint8_t adc_need_deinit() {
  uint32_t sum=0;
  for (uint32_t k=0; k<ADC_NUM; k++) {
    sum = sum + adc_configured[k];
  }  
  return (sum==0) ? 1:0;
}

/* ADC1 init function */
static void adc1_init() {
  ADC_ChannelConfTypeDef sConfig;

  /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
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
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  for (int k=0; k<ADC_NUM; k++) {
    sConfig.Channel = adc_config[k].channel;
    sConfig.Rank = k+1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }
  }
}

/** 
*/
static void adc1_deinit() {
  HAL_ADC_DeInit(&hadc1);
}

/**
 * Read value from the ADC channel
 * 
 * @param {uint8_t} pin
 * @return {double}
 */
double adc_read(uint8_t pin) {
  uint8_t n = get_adc_index(pin);
  return (double)adc_buf[n] / (1 << ADC_RESOLUTION_BIT);
}

/**
 * Setup a ADC channel
 * 
 * @param {uint8_t} pin
 * @return result status code
 */
int adc_setup(uint8_t pin) {
  if (adc_need_init()) {
    adc1_init();
    adc1_start_dma();
  }

  uint8_t n = get_adc_index(pin);
  GPIO_InitTypeDef GPIO_InitStruct;
  GPIO_InitStruct.Pin = adc_config[n].pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(adc_config[n].port, &GPIO_InitStruct);

  adc_configured[n] = 1;
  return 0;
}

/**
 * Close the ADC channel
 */
void adc_close(uint8_t pin) {
  uint8_t n = get_adc_index(pin);
  HAL_GPIO_DeInit(adc_config[n].port, adc_config[n].pin);
  adc_configured[n] = 0;
  
  if (adc_need_deinit()) {  
    adc1_deinit();
  }
}

void adc_test()
{
#if 0  
   adc_setup(23);
   delay(1);
   while(1)
   {
          double val = adc_read( 23 );
          printf("%f \r\n", val);
          delay(1000);
   }
#endif  
  
   // 
   uint8_t n=0;
   adc_setup(adc_config[n].pin_number); n++;
   adc_setup(adc_config[n].pin_number); n++;
   adc_setup(adc_config[n].pin_number); n++;
   adc_setup(adc_config[n].pin_number); n++;
   adc_setup(adc_config[n].pin_number); n++;
   adc_setup(adc_config[n].pin_number); n++;
   adc_setup(adc_config[n].pin_number); n++;
   adc_setup(adc_config[n].pin_number); n++;

   delay(1);

   for(int k=0; k<1000; k++)
   {
      for(int m=0; m<n; m++) 
      {
          double val = adc_read( adc_config[m].pin_number );
          tty_printf("%f \r\n", val);
          printf("%f \r\n", val);
      }
      tty_printf("\r\n\n");
      printf("\r\n\n");
      
      delay(1000);
   }

   n=0;
   adc_close(adc_config[n].pin_number); n++;
   adc_close(adc_config[n].pin_number); n++;
   adc_close(adc_config[n].pin_number); n++;
   adc_close(adc_config[n].pin_number); n++;
   adc_close(adc_config[n].pin_number); n++;
   adc_close(adc_config[n].pin_number); n++;
   adc_close(adc_config[n].pin_number); n++;
   
   while(1);
}