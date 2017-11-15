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

#include "system.h"
#include "gpio.h"
#include "tty.h"
#include "io.h"
#include "runtime.h"

void timer_cb() {
  gpio_toggle(4); // LED Blinking
  /* Should not be blocked */
  uint64_t tick = gettime();
  tty_printf("[%d] : LED blinking...\r\n", (int) tick);
}

void tty_read_cb(char ch) {
  /* echo */
  tty_putc(ch);
}

int main(void) {
  system_init();
  tty_init();
  io_init();

  gpio_pin_mode(4, GPIO_PIN_MODE_OUPUT_PP);

  /* Timer setup */
  io_timer_handle_t timer;
  io_timer_init(&timer);
  io_timer_start(&timer, timer_cb, 1000, true);

  /* TTY setup */
  /*
  io_tty_handle_t tty;
  io_tty_init(&tty);
  io_tty_read_start(&tty, tty_read_cb);
  */

  // runtime_test();


  /* Enter to IO loop */
  io_run();
}


/**
 * @brief  This function is executed in case of error occurrence.
 * @param  None
 * @retval None
 */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}
