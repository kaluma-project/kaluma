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
#include "stm32f4xx.h"

#define NUM_ADC_CHANNEL 4

DMA_HandleTypeDef hdma_adc1;
static uint16_t adc_buf[NUM_ADC_CHANNEL];
static uint8_t adc_configured[NUM_ADC_CHANNEL];
static ADC_HandleTypeDef hadc1;
static struct {
    GPIO_TypeDef * port;
    uint32_t pin;
} adc_port_pin[] = {
   {GPIOC, GPIO_PIN_1},     // 0
   {GPIOC, GPIO_PIN_5},     // 1
   {GPIOC, GPIO_PIN_4},     // 2
   {GPIOB, GPIO_PIN_0},     // 3
};

/** 
*/
static void adc1_start_dma() {
  HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_buf, NUM_ADC_CHANNEL);
}

/** 
*/
static void adc_pin_enable(uint8_t pin) {
  assert_param(pin < NUM_ADC_CHANNEL);
  GPIO_InitTypeDef GPIO_InitStruct;

  /**ADC1 GPIO Configuration    
  PC1     ------> ADC1_IN11
  PC5     ------> ADC1_IN15
  PC4     ------> ADC1_IN14
  PB0     ------> ADC1_IN8 
  */
  GPIO_InitStruct.Pin = adc_port_pin[pin].pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(adc_port_pin[pin].port, &GPIO_InitStruct);
  
  adc_configured[pin] = 1;
}

/** 
*/
static void adc_pin_disable(uint8_t pin) {
  assert_param(pin < NUM_ADC_CHANNEL);
  HAL_GPIO_DeInit(adc_port_pin[pin].port, adc_port_pin[pin].pin);
  adc_configured[pin] = 0;
}

/** 
*/
static uint8_t adc_need_init() {
  uint32_t sum=0;
  for (uint32_t k=0; k<NUM_ADC_CHANNEL; k++) {
    sum = sum + adc_configured[k];
  }  
  return (sum==0) ? 1:0;
}

/** 
*/
static uint8_t adc_need_deinit() {
  uint32_t sum=0;
  for (uint32_t k=0; k<NUM_ADC_CHANNEL; k++) {
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
  hadc1.Init.NbrOfConversion = NUM_ADC_CHANNEL;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
  */
  sConfig.Channel = ADC_CHANNEL_15;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
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
 * @return {uint32_t}
 */
uint32_t adc_read(uint8_t pin) {
  assert_param(pin < NUM_ADC_CHANNEL);
  return (uint32_t)adc_buf[pin];
}

/**
 * Setup a ADC channel
 * 
 * @param {uint8_t} pin
 * @return result status code
 */
int adc_setup(uint8_t pin) {
  assert_param(pin < NUM_ADC_CHANNEL);

  if (adc_need_init()) {
    adc1_init();
    adc1_start_dma();
  }
  
  adc_pin_enable(pin);
  return 0;
}

/**
 * Close the ADC channel
 */
void adc_close(uint8_t pin) {
  assert_param(pin < NUM_ADC_CHANNEL);
  adc_pin_disable(pin);  

  if (adc_need_deinit()) {  
    adc1_deinit();
  }
}

void adc_test()
{
   adc_setup(0);
   adc_setup(1);
   adc_setup(2);
   adc_setup(3);
   for(int k=0; k<1000; k++)
   {
      tty_printf("%d \r\n", adc_read(0));
      printf("%d \r\n", adc_read(0));
      
      tty_printf("%d \r\n", adc_read(1));
      printf("%d \r\n", adc_read(1));

      tty_printf("%d \r\n", adc_read(2));
      printf("%d \r\n", adc_read(2));

      tty_printf("%d \r\n\n", adc_read(3));
      printf("%d \r\n\n", adc_read(3));

      delay(1000);
   }
   adc_close(0);
   adc_close(1);
   adc_close(2);
   adc_close(3);
   
   while(1);
}
