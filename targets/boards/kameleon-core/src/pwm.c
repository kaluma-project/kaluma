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

extern void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

static void tim1_pwm_setup(uint32_t, uint32_t, uint32_t, uint32_t);
static void tim2_pwm_setup(uint32_t, uint32_t, uint32_t, uint32_t);
static void tim3_pwm_setup(uint32_t, uint32_t, uint32_t, uint32_t);
static void tim4_pwm_setup(uint32_t, uint32_t, uint32_t, uint32_t);
static void tim5_pwm_setup(uint32_t, uint32_t, uint32_t, uint32_t);

static TIM_HandleTypeDef htim1;
static TIM_HandleTypeDef htim2;
static TIM_HandleTypeDef htim3;
static TIM_HandleTypeDef htim4;
static TIM_HandleTypeDef htim5;

#define PWMMAXFREQ    450000 //450kHz
#define PWMMINFREQ    1      //1Hz
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
    void (*setup)(uint32_t, uint32_t, uint32_t, uint32_t);
    HAL_StatusTypeDef (*start)(TIM_HandleTypeDef *, uint32_t);
    HAL_StatusTypeDef (*stop)(TIM_HandleTypeDef *, uint32_t);
    uint8_t bus;
} pwm_config[] = {
   {1, GPIOB, GPIO_PIN_1, &htim1, TIM1, TIM_CHANNEL_3, tim1_pwm_setup, HAL_TIMEx_OCN_Start, HAL_TIMEx_OCN_Stop, APB2},
   {2, GPIOA, GPIO_PIN_0, &htim5, TIM5, TIM_CHANNEL_1, tim5_pwm_setup, HAL_TIM_PWM_Start, HAL_TIM_PWM_Stop, APB1},
   {14, GPIOB, GPIO_PIN_5, &htim3, TIM3, TIM_CHANNEL_2, tim3_pwm_setup, HAL_TIM_PWM_Start, HAL_TIM_PWM_Stop, APB1},
   {15, GPIOB, GPIO_PIN_10, &htim2, TIM2, TIM_CHANNEL_3, tim2_pwm_setup, HAL_TIM_PWM_Start, HAL_TIM_PWM_Stop, APB1},
   {16, GPIOB, GPIO_PIN_9, &htim4, TIM4, TIM_CHANNEL_4, tim4_pwm_setup, HAL_TIM_PWM_Start, HAL_TIM_PWM_Stop, APB1},
};

/**
 * Get PWM index
 *
 * @param pin Pin number.
 * @return Returns index number on success or 255 on failure.
 */
/**
 * input : pinNumber
 * output : pinIndex or 0xFF
 *          0xFF means the pin is not assigned for PWM
*/
static uint8_t get_pwm_index(uint8_t pin) {
  uint32_t n = sizeof(pwm_config) / sizeof(struct __pwm_config);
  uint8_t index = 0xFF;

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
  uint8_t bus;
  //bus = pwm_config[n].bus;
  bus = APB2; //Workaround, all Timer clock seems to be 96MHz.
  if (bus==APB1) {
    tick_freq = HAL_RCC_GetPCLK1Freq();
  } else if (bus==APB2) {
    tick_freq = HAL_RCC_GetPCLK2Freq();
  }
  return tick_freq;
}

/**
*/
static uint32_t get_tick_prescaler(uint8_t n, double frequency) {
  uint32_t prescaler;
  uint8_t bus;
  //bus = pwm_config[n].bus;
  bus = APB2; //Workaround, all Timer clock seems to be 96MHz.
  if (bus == APB1) { //48MHz
    if (frequency > 700)
      prescaler = 1;
    else if (frequency > 100)
      prescaler = 8;
    else if (frequency > 15)
      prescaler = 64;
    else
      prescaler = 1024;
  } else if (bus == APB2) { //96MHz
    if (frequency > 1500)
      prescaler = 1;
    else if (frequency > 200)
      prescaler = 8;
    else if (frequency > 25)
      prescaler = 64;
    else if (frequency > 1.5)
      prescaler = 1024;
    else
      prescaler = 2048;
  }
  return prescaler;
}

/**
*/
static void tim1_pwm_setup(uint32_t channel, uint32_t prescaler, uint32_t arr, uint32_t pulse) {
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = prescaler - 1;
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
static void tim2_pwm_setup(uint32_t channel, uint32_t prescaler, uint32_t arr, uint32_t pulse) {
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = prescaler - 1;
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
static void tim3_pwm_setup(uint32_t channel, uint32_t prescaler, uint32_t arr, uint32_t pulse) {
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = prescaler - 1;
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
static void tim4_pwm_setup(uint32_t channel, uint32_t prescaler, uint32_t arr, uint32_t pulse) {
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = prescaler - 1;
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
static void tim5_pwm_setup(uint32_t channel, uint32_t prescaler, uint32_t arr, uint32_t pulse) {
  TIM_MasterConfigTypeDef sMasterConfig;
  TIM_OC_InitTypeDef sConfigOC;

  htim5.Instance = TIM5;
  htim5.Init.Prescaler = prescaler - 1;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = arr;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_PWM_Init(&htim5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = pulse;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, channel) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  HAL_TIM_MspPostInit(&htim5);
}

/**
 * return Returns 0 on success or -1 on failure.
*/

int pwm_setup(uint8_t pin, double frequency, double duty) {
  uint32_t tick_freq, ch, prescaler, arr, pulse;
  uint8_t pduty = (duty + 0.005f) * 100;
  uint8_t n = get_pwm_index(pin);
  if (n == 0xFF) //Not a index of PWM pin
    return -1;

  //Mix Max check
  if (pduty > 100)
    pduty = 100;
  if (frequency > PWMMAXFREQ) //Max
    frequency = PWMMAXFREQ;
  else if (frequency < PWMMINFREQ) //Min
    frequency = PWMMINFREQ;

  tick_freq = get_tick_frequency(n);
  prescaler = get_tick_prescaler(n, frequency);
  ch = pwm_config[n].channel;
  arr = (uint32_t)(tick_freq / prescaler / frequency);
  pulse = (uint32_t)((pduty * arr)/100);

  pwm_config[n].setup(ch, prescaler, arr, pulse);

  return 0;
}

/**
*/
void pwm_start(uint8_t pin) {
  uint8_t n = get_pwm_index(pin);
  if (n != 0xFF) //PWM pin index
    pwm_config[n].start(pwm_config[n].handle, pwm_config[n].channel);
}

/**
*/
void pwm_stop(uint8_t pin) {
  uint8_t n = get_pwm_index(pin);
  if (n != 0xFF) //PWM pin index
    pwm_config[n].stop(pwm_config[n].handle, pwm_config[n].channel);
}

/**
*/
double pwm_get_frequency(uint8_t pin) {
  uint8_t n = get_pwm_index(pin);
  if (n != 0xFF) { //PWM pin index
    uint32_t tick_freq = get_tick_frequency(n);
    return (double)tick_freq/(pwm_config[n].handle->Init.Prescaler + 1)/(__HAL_TIM_GET_AUTORELOAD(pwm_config[n].handle) + 1);
  }
}

/**
*/
double pwm_get_duty(uint8_t pin) {
  uint8_t n = get_pwm_index(pin);
  if (n != 0xFF) { //PWM pin index
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(pwm_config[n].handle);
    uint32_t pulse = __HAL_TIM_GET_COMPARE(pwm_config[n].handle, pwm_config[n].channel);
    return (double)(pulse)/(arr);
  }
  return -1;
}

/**
*/
void pwm_set_duty(uint8_t pin, double duty) {
  uint8_t n = get_pwm_index(pin);
  if (n != 0xFF) { //PWM pin index
    while (__HAL_TIM_GET_COUNTER(pwm_config[n].handle) != 0);
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(pwm_config[n].handle);
    uint32_t pulse = (uint32_t)(duty * arr);
    __HAL_TIM_SET_COMPARE(pwm_config[n].handle, pwm_config[n].channel, pulse);
  }
}

/**
*/
void pwm_set_frequency(uint8_t pin, double frequency) {
  uint8_t n = get_pwm_index(pin);
  if (n != 0xFF) { //PWM pin index
    double previous_duty = pwm_get_duty(pin);
    /* The previous duty ratio must be hold up regardless of changing frequency */
    while (__HAL_TIM_GET_COUNTER(pwm_config[n].handle) != 0);
    pwm_setup(pin, frequency, previous_duty);
    pwm_config[n].start(pwm_config[n].handle, pwm_config[n].channel);
  }
}

/**
*/
void pwm_close(uint8_t pin) {
  uint8_t n = get_pwm_index(pin);
  if (n != 0xFF) //PWM pin index
    HAL_TIM_PWM_DeInit(pwm_config[n].handle);
}
