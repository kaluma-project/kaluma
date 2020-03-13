#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"

#include "system.h"
#include "gpio.h"
#include "tty.h"
#include "io.h"
#include "repl.h"
#include "runtime.h"

static const char* TAG = "kameleon";

void app_main()
{
  bool load = false;
  /* Print chip information */
  system_init();
  load = running_script_check();
  tty_init();
  io_init();
  repl_init();
  runtime_init(load, true);
  io_run();
}
