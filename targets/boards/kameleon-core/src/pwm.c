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
#include "kameleon_core.h"

static void tim2_pwm_setup(uint32_t, uint32_t, uint32_t);
static void tim3_pwm_setup(uint32_t, uint32_t, uint32_t);
static void tim4_pwm_setup(uint32_t, uint32_t, uint32_t);

static TIM_HandleTypeDef htim2;
static TIM_HandleTypeDef htim3;
static TIM_HandleTypeDef htim4;

/**
* APB1 BUS CLOCK (48MHz max)
* APB1 TIMER CLOCK (96MHz max)
*
* APB2 BUS CLOCK (96MHz max)
* APB2 TIMER CLOCK (96MHz max)
*/
static const struct __pwm_config {
    uint8_t pin_number;
    GPIO_TypeDef * port;
    uint32_t pin;
    TIM_HandleTypeDef * handle;
    TIM_TypeDef * instance;
    uint32_t channel;
    void (*setup)(uint32_t, uint32_t, uint32_t);
    uint8_t bus;
} pwm_config[] = {
   {13, GPIOB, GPIO_PIN_4, &htim3, TIM3, TIM_CHANNEL_1, tim3_pwm_setup, APB1},
   {14, GPIOB, GPIO_PIN_5, &htim3, TIM3, TIM_CHANNEL_2, tim3_pwm_setup, APB1},
   {15, GPIOB, GPIO_PIN_10, &htim2, TIM2, TIM_CHANNEL_3, tim2_pwm_setup, APB1},
   {16, GPIOB, GPIO_PIN_9, &htim4, TIM4, TIM_CHANNEL_4, tim4_pwm_setup, APB1},
};

/**
* tick freq division (must be greater than or equal to 1)
*/
static const uint32_t tick_freq_div=4;


/**
*/
static uint8_t get_pwm_index(uint8_t pin) {
  assert_param(IS_PWM_PINS(pin));

  uint32_t n = sizeof(pwm_config) / sizeof(struct __pwm_config);
  uint8_t index;

  for (int k=0; k<n; k++) {
    if (pwm_config[k].pin_number == pin) {
      index = k;
      break;
    }
  }  
  
  return index;    
}

/**
*/
static uint32_t get_tick_frequency(uint8_t n) {
  uint32_t tick_freq;
  uint8_t bus = pwm_config[n].bus;
  
  if (bus==APB1) {
    tick_freq = 2 * HAL_RCC_GetPCLK1Freq();
  } else if (bus==APB2) {
    tick_freq = 2 * HAL_RCC_GetPCLK2Freq();
  }  
  return (tick_freq / tick_freq_div);
}

/**
*/
static void tim2_pwm_setup(uint32_t channel, uint32_t arr, uint32_t pulse) {
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = tick_freq_div - 1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = arr;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = pulse;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, channel) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim2);

}

/**
*/
static void tim3_pwm_setup(uint32_t channel, uint32_t arr, uint32_t pulse) {
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = tick_freq_div - 1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = arr;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = pulse;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, channel) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim3);

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
int pwm_setup(uint8_t pin, double frequency, double duty) {
  uint32_t tick_freq, ch, arr, pulse;
  uint8_t n = get_pwm_index(pin);

  tick_freq = get_tick_frequency(n);
  ch = pwm_config[n].channel;
  arr = (uint32_t)(tick_freq / frequency + 0.5f) - 1;
  pulse = (uint32_t)(duty * (arr + 1) + 0.5f);
  pwm_config[n].setup(ch, arr, pulse);
  
  return 0;
}

/**
*/
void pwm_start(uint8_t pin) {
  uint8_t n = get_pwm_index(pin);
  HAL_TIM_PWM_Start(pwm_config[n].handle, pwm_config[n].channel);
}

/**
*/
void pwm_stop(uint8_t pin) {
  uint8_t n = get_pwm_index(pin);
  HAL_TIM_PWM_Stop(pwm_config[n].handle, pwm_config[n].channel);
}

/**
*/
double pwm_get_frequency(uint8_t pin) {
  uint8_t n = get_pwm_index(pin);
  uint32_t tick_freq = get_tick_frequency(n);
  return (double)tick_freq/(__HAL_TIM_GET_AUTORELOAD(pwm_config[n].handle)+1);
}

/**
*/
double pwm_get_duty(uint8_t pin) {
  uint8_t n = get_pwm_index(pin);
  uint32_t arr = __HAL_TIM_GET_AUTORELOAD(pwm_config[n].handle);
  uint32_t pulse = __HAL_TIM_GET_COMPARE(pwm_config[n].handle, pwm_config[n].channel);
  return (double)(pulse)/(arr+1);
}

/**
*/
void pwm_set_duty(uint8_t pin, double duty) {
  uint8_t n = get_pwm_index(pin);
  uint32_t arr = __HAL_TIM_GET_AUTORELOAD(pwm_config[n].handle);
  uint32_t pulse = (uint32_t)(duty * (arr + 1) + 0.5f);  
  __HAL_TIM_SET_COMPARE(pwm_config[n].handle, pwm_config[n].channel, pulse);
}

/**
*/
void pwm_set_frequency(uint8_t pin, double frequency) {
  double previous_duty = pwm_get_duty(pin);

  uint8_t n = get_pwm_index(pin);
  uint32_t arr = (uint32_t)(get_tick_frequency(n)/frequency + 0.5f) - 1;
  uint32_t pulse = (uint32_t)(previous_duty * (arr + 1) + 0.5f);  
  
  /* The previous duty ratio must be hold up regardless of changing frequency */
  __HAL_TIM_SET_COMPARE(pwm_config[n].handle, pwm_config[n].channel, pulse);
  __HAL_TIM_SET_AUTORELOAD(pwm_config[n].handle, arr);
  
  /* The counter value must be reset for not being over the arr value. */
  __HAL_TIM_SET_COUNTER(pwm_config[n].handle, 0);
}    

/**
*/
void pwm_close(uint8_t pin) {
  uint8_t n = get_pwm_index(pin);
  HAL_TIM_PWM_DeInit(pwm_config[n].handle);  
}


void pwm_test()
{  
  /*
  * 13번, 14번은 TIM3에 같이 연결되어 있으므로 PWM 주파수 설정을 다르게 할 수 없음
  * 마지막에 설정한 주파수가 반영됨
  */
  pwm_setup(13, 1000, 0.10);
  pwm_setup(14, 2000, 0.20);
  pwm_setup(15, 4000, 0.40);
  pwm_setup(16, 8000, 0.80);

  pwm_set_duty(13, 0.1);
  pwm_set_duty(14, 0.2);
  
  pwm_start(13);
  pwm_start(14);
  pwm_start(15);
  pwm_start(16);

  printf("%f \r\n", pwm_get_frequency(13));
  printf("%f \r\n", pwm_get_frequency(14));
  printf("%f \r\n", pwm_get_frequency(15));
  printf("%f \r\n", pwm_get_frequency(16));
  
  printf("%f \r\n", pwm_get_duty(13));
  printf("%f \r\n", pwm_get_duty(14));
  printf("%f \r\n", pwm_get_duty(15));
  printf("%f \r\n", pwm_get_duty(16));

  delay(10000);

  pwm_set_frequency(13, 10000);
  pwm_set_frequency(14, 20000);
  pwm_set_frequency(15, 40000);
  pwm_set_frequency(16, 80000);
  
  pwm_set_duty(13, 0.8);
  pwm_set_duty(14, 0.4);
  pwm_set_duty(15, 0.2);
  pwm_set_duty(16, 0.1);

  printf("%f \r\n", pwm_get_frequency(13));
  printf("%f \r\n", pwm_get_frequency(14));
  printf("%f \r\n", pwm_get_frequency(15));
  printf("%f \r\n", pwm_get_frequency(16));
  
  printf("%f \r\n", pwm_get_duty(13));
  printf("%f \r\n", pwm_get_duty(14));
  printf("%f \r\n", pwm_get_duty(15));
  printf("%f \r\n", pwm_get_duty(16));
    
  delay(10000);

  pwm_stop(13);
  pwm_stop(14);
  pwm_stop(15);
  pwm_stop(16);
  
  pwm_close(13);
  pwm_close(14);
  pwm_close(15);
  pwm_close(16);
  
  while(1);
  
}
