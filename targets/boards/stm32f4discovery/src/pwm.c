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

static void tim1_pwm_setup(uint32_t, uint32_t, uint32_t);
static void tim2_pwm_setup(uint32_t, uint32_t, uint32_t);
static void tim3_pwm_setup(uint32_t, uint32_t, uint32_t);
static void tim4_pwm_setup(uint32_t, uint32_t, uint32_t);
static void tim8_pwm_setup(uint32_t, uint32_t, uint32_t);
static void tim9_pwm_setup(uint32_t, uint32_t, uint32_t);
static void tim10_pwm_setup(uint32_t, uint32_t, uint32_t);
static void tim12_pwm_setup(uint32_t, uint32_t, uint32_t);

static TIM_HandleTypeDef htim1;
static TIM_HandleTypeDef htim2;
static TIM_HandleTypeDef htim3;
static TIM_HandleTypeDef htim4;
static TIM_HandleTypeDef htim8;
static TIM_HandleTypeDef htim9;
static TIM_HandleTypeDef htim10;
static TIM_HandleTypeDef htim12;

/**
* APB1 BUS CLOCK (42MHz max)
* APB1 TIMER CLOCK (84MHz max)
*
* APB2 BUS CLOCK (84MHz max)
* APB2 TIMER CLOCK (168MHz max)
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
   {0, GPIOC, GPIO_PIN_6, &htim3, TIM3, TIM_CHANNEL_1, tim3_pwm_setup, APB1},
   {2, GPIOC, GPIO_PIN_9, &htim8, TIM8, TIM_CHANNEL_4, tim8_pwm_setup, APB2},
   {4, GPIOA, GPIO_PIN_15, &htim2, TIM2, TIM_CHANNEL_1, tim2_pwm_setup, APB1},
   {14, GPIOB, GPIO_PIN_7, &htim4, TIM4, TIM_CHANNEL_2, tim4_pwm_setup, APB1},
   {15, GPIOB, GPIO_PIN_8, &htim10, TIM10, TIM_CHANNEL_1, tim10_pwm_setup, APB2},
   {18, GPIOE, GPIO_PIN_5, &htim9, TIM9, TIM_CHANNEL_1, tim9_pwm_setup, APB2},
   {35, GPIOE, GPIO_PIN_9, &htim1, TIM1, TIM_CHANNEL_1, tim1_pwm_setup, APB2},
   {45, GPIOB, GPIO_PIN_14, &htim12, TIM12, TIM_CHANNEL_1, tim12_pwm_setup, APB1},
};

/**
* tick freq division (must be greater than or equal to 1)
*/
static const uint32_t tick_freq_div=4;


/**
*/
static uint8_t get_pwm_index(uint8_t pin) {
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
static void tim1_pwm_setup(uint32_t channel, uint32_t arr, uint32_t pulse) {
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = tick_freq_div - 1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = arr;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = pulse;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, channel) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim1);
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
static void tim8_pwm_setup(uint32_t channel, uint32_t arr, uint32_t pulse) {
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

  htim8.Instance = TIM8;
  htim8.Init.Prescaler = tick_freq_div - 1;
  htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim8.Init.Period = arr;
  htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim8.Init.RepetitionCounter = 0;
  if (HAL_TIM_PWM_Init(&htim8) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = pulse;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim8, &sConfigOC, channel) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim8);

}

/**
*/
static void tim9_pwm_setup(uint32_t channel, uint32_t arr, uint32_t pulse) {
  TIM_OC_InitTypeDef sConfigOC;

  htim9.Instance = TIM9;
  htim9.Init.Prescaler = tick_freq_div - 1;
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
  pwm_setup(2, 21000, 0);
  pwm_start(2);
  
  delay(10000);
  
  pwm_setup(0, 21000, 0.50);
  pwm_setup(2, 2000, 0.50);
  pwm_setup(4, 4000, 0.50);
  pwm_setup(14, 8000, 0.40);
  pwm_setup(15, 16000, 0.50);
  pwm_setup(18, 32000, 0.60);
  pwm_setup(35, 64000, 0.70);
  pwm_setup(45, 128000, 0.80);

  pwm_start(0);
  pwm_start(2);
  pwm_start(4);
  pwm_start(14);
  pwm_start(15);
  pwm_start(18);
  pwm_start(35);
  pwm_start(45);

  delay(10000);
  
  printf("%f \r\n", pwm_get_frequency(0));
  printf("%f \r\n", pwm_get_frequency(2));
  printf("%f \r\n", pwm_get_frequency(4));
  printf("%f \r\n", pwm_get_frequency(14));
  printf("%f \r\n", pwm_get_frequency(15));
  printf("%f \r\n", pwm_get_frequency(18));
  printf("%f \r\n", pwm_get_frequency(35));
  printf("%f \r\n\n", pwm_get_frequency(45));
  
  printf("%f \r\n", pwm_get_duty(0));
  printf("%f \r\n", pwm_get_duty(2));
  printf("%f \r\n", pwm_get_duty(4));
  printf("%f \r\n", pwm_get_duty(14));
  printf("%f \r\n", pwm_get_duty(15));
  printf("%f \r\n", pwm_get_duty(18));
  printf("%f \r\n", pwm_get_duty(35));
  printf("%f \r\n\n", pwm_get_duty(45));
  
  pwm_set_frequency(0, 21000);
  pwm_set_frequency(2, 14000);
  pwm_set_frequency(4, 21000);
  pwm_set_frequency(14, 28000);
  pwm_set_frequency(15, 35000);
  pwm_set_frequency(18, 42000);
  pwm_set_frequency(35, 49000);
  pwm_set_frequency(45, 56000);

  pwm_set_duty(0, 0.50);
  pwm_set_duty(2, 0.35);
  pwm_set_duty(4, 0.45);
  pwm_set_duty(14, 0.55);
  pwm_set_duty(15, 0.65);
  pwm_set_duty(18, 0.75);
  pwm_set_duty(35, 0.85);
  pwm_set_duty(45, 0.95);

  printf("%f \r\n", pwm_get_frequency(0));
  printf("%f \r\n", pwm_get_frequency(2));
  printf("%f \r\n", pwm_get_frequency(4));
  printf("%f \r\n", pwm_get_frequency(14));
  printf("%f \r\n", pwm_get_frequency(15));
  printf("%f \r\n", pwm_get_frequency(18));
  printf("%f \r\n", pwm_get_frequency(35));
  printf("%f \r\n\n", pwm_get_frequency(45));
  
  printf("%f \r\n", pwm_get_duty(0));
  printf("%f \r\n", pwm_get_duty(2));
  printf("%f \r\n", pwm_get_duty(4));
  printf("%f \r\n", pwm_get_duty(14));
  printf("%f \r\n", pwm_get_duty(15));
  printf("%f \r\n", pwm_get_duty(18));
  printf("%f \r\n", pwm_get_duty(35));
  printf("%f \r\n\n", pwm_get_duty(45));
    
  pwm_stop(0);
  pwm_stop(2);
  pwm_stop(4);
  pwm_stop(14);
  pwm_stop(15);
  pwm_stop(18);
  pwm_stop(35);
  pwm_stop(45);
  
  pwm_close(0);
  pwm_close(2);
  pwm_close(4);
  pwm_close(14);
  pwm_close(15);
  pwm_close(18);
  pwm_close(35);
  pwm_close(45);
  
  while(1);
  
}
