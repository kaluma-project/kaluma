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

static void tim4_pwm_setup(uint32_t, uint32_t, uint32_t);
static void tim9_pwm_setup(uint32_t, uint32_t, uint32_t);
static void tim10_pwm_setup(uint32_t, uint32_t, uint32_t);
static void tim12_pwm_setup(uint32_t, uint32_t, uint32_t);

static TIM_HandleTypeDef htim4;
static TIM_HandleTypeDef htim9;
static TIM_HandleTypeDef htim10;
static TIM_HandleTypeDef htim12;

/**
* APB1 BUS CLOCK (42MHz max)
* APB1 TIMER CLOCK (84MHz max)
*
* APB2 BUS CLOCK (84MHz max)
* APB2 TIMER CLOCK (168MHz max)
*
* TIM4 (APB1)
* TIM9 (APB2)
* TIM10 (APB2)
* TIM12 (APB1)
*/
static const struct {
    GPIO_TypeDef * port;
    uint32_t pin;
    TIM_HandleTypeDef * handle;
    TIM_TypeDef * instance;
    uint32_t channel;
    void (*setup)(uint32_t, uint32_t, uint32_t);
} pwm_config[] = {
   {GPIOB, GPIO_PIN_7, &htim4, TIM4, TIM_CHANNEL_2, tim4_pwm_setup},           // 0
   {GPIOB, GPIO_PIN_8, &htim10, TIM10, TIM_CHANNEL_1, tim10_pwm_setup},        // 1
   {GPIOE, GPIO_PIN_5, &htim9, TIM9, TIM_CHANNEL_1, tim9_pwm_setup},           // 2
   {GPIOB, GPIO_PIN_14, &htim12, TIM12, TIM_CHANNEL_1, tim12_pwm_setup},       // 3
};

/**
* tick freq division (must be greater than or equal to 1)
*/
static const uint32_t tick_freq_div=4;


/**
*/
static uint32_t get_tick_frequency(uint8_t pin) {
  uint32_t tick_freq;
  TIM_TypeDef * instance = pwm_config[pin].instance;

  if (instance==TIM4 || instance==TIM12) {
    tick_freq = 2 * HAL_RCC_GetPCLK1Freq();
  } else if (instance==TIM9 || instance==TIM10) {
    tick_freq = 2 * HAL_RCC_GetPCLK2Freq();
  }  
  return (tick_freq / tick_freq_div);
}

/**
*/
static void tim4_pwm_setup(uint32_t channel, uint32_t arr, uint32_t pulse) {
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = tick_freq_div - 1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = arr;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_PWM_Init(&htim4) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = pulse;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, channel) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim4);
}

/**
*/
static void tim9_pwm_setup(uint32_t channel, uint32_t arr, uint32_t pulse) {
  TIM_OC_InitTypeDef sConfigOC;

  htim9.Instance = TIM9;
  htim9.Init.Prescaler = tick_freq_div - 1;;
  htim9.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim9.Init.Period = arr;
  htim9.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_PWM_Init(&htim9) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = pulse;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim9, &sConfigOC, channel) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim9);
}

/**
*/
static void tim10_pwm_setup(uint32_t channel, uint32_t arr, uint32_t pulse) {
  TIM_OC_InitTypeDef sConfigOC;

  htim10.Instance = TIM10;
  htim10.Init.Prescaler = tick_freq_div - 1;;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = arr;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  if (HAL_TIM_PWM_Init(&htim10) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = pulse;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim10, &sConfigOC, channel) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim10);
}

/**
*/
static void tim12_pwm_setup(uint32_t channel, uint32_t arr, uint32_t pulse) {
  TIM_OC_InitTypeDef sConfigOC;

  htim12.Instance = TIM12;
  htim12.Init.Prescaler = tick_freq_div - 1;;
  htim12.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim12.Init.Period = arr;
  htim12.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_PWM_Init(&htim12) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = pulse;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim12, &sConfigOC, channel) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim12);
}

/**
*/
int pwm_setup(uint8_t pin, uint32_t frequency, uint32_t duty) {
  assert_param(pin<4);
  uint32_t tick_freq, ch, arr, pulse;

  tick_freq = get_tick_frequency(pin);
  ch = pwm_config[pin].channel;
  arr = (uint32_t)((float)tick_freq / frequency - 1 + 0.5f);
  pulse = (uint32_t)((duty / 100.0f) * arr);
  pwm_config[pin].setup(ch, arr, pulse);
  
  return 0;
}

/**
*/
void pwm_start(uint8_t pin) {
  HAL_TIM_PWM_Start(pwm_config[pin].handle, pwm_config[pin].channel);
}

/**
*/
void pwm_stop(uint8_t pin) {
  HAL_TIM_PWM_Stop(pwm_config[pin].handle, pwm_config[pin].channel);
}

/**
*/
uint32_t pwm_get_frequency(uint8_t pin) {
  assert_param(pin<4);  
  float tick_freq = (float)get_tick_frequency(pin);
  return (uint32_t)(tick_freq/(__HAL_TIM_GET_AUTORELOAD(pwm_config[pin].handle)+1) + 0.5f);
}

/**
*/
uint32_t pwm_get_duty(uint8_t pin) {
  
  uint32_t arr = __HAL_TIM_GET_AUTORELOAD(pwm_config[pin].handle);
  uint32_t pulse = __HAL_TIM_GET_COMPARE(pwm_config[pin].handle, pwm_config[pin].channel);
  return (uint32_t)(100.0f * pulse / (float)arr + 0.5f);
}

/**
*/
void pwm_set_frequency(uint8_t pin, uint32_t frequency) {
  assert_param(pin<4);
  
  uint32_t arr = (uint32_t)((float)get_tick_frequency(pin)/frequency - 1 + 0.5f);
  __HAL_TIM_SET_AUTORELOAD(pwm_config[pin].handle, arr);
}    

/**
*/
void pwm_set_duty(uint8_t pin, uint32_t duty) {
  assert_param(pin<4);

  uint32_t arr = __HAL_TIM_GET_AUTORELOAD(pwm_config[pin].handle);
  uint32_t pulse = (uint32_t)(duty / 100.0f * (float)arr + 0.5f);  
  __HAL_TIM_SET_COMPARE(pwm_config[pin].handle, pwm_config[pin].channel, pulse);
}

/**
*/
void pwm_close(uint8_t pin) {
  HAL_TIM_PWM_DeInit(pwm_config[pin].handle);  
}


void pwm_test()
{
  pwm_setup(0, 1000, 10);
  pwm_setup(1, 5000, 25);
  pwm_setup(2, 10000, 50);
  pwm_setup(3, 50000, 75);

  pwm_start(0);
  pwm_start(1);
  pwm_start(2);
  pwm_start(3);

  delay(10000);
  
  tty_printf("%d \r\n", pwm_get_frequency(0));
  tty_printf("%d \r\n", pwm_get_frequency(1));
  tty_printf("%d \r\n", pwm_get_frequency(2));
  tty_printf("%d \r\n\n", pwm_get_frequency(3));
  
  tty_printf("%d \r\n", pwm_get_duty(0));
  tty_printf("%d \r\n", pwm_get_duty(1));
  tty_printf("%d \r\n", pwm_get_duty(2));
  tty_printf("%d \r\n\n", pwm_get_duty(3));
  
  
  pwm_stop(0);
  pwm_stop(1);
  pwm_stop(2);
  pwm_stop(3);
  
  pwm_close(0);
  pwm_close(1);
  pwm_close(2);
  pwm_close(3);
  
  while(1);
  
}
