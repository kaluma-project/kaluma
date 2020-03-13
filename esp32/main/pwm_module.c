#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "jerryscript.h"
#include "jerryscript-ext/handler.h"
#include "runtime.h"
#include "global.h"
#include "jerryxx.h"

extern jerry_value_t module_pwm_init();

void register_global_pwm()
{
  jerry_value_t global = jerry_get_global_object();
  jerry_value_t pwm_module = module_pwm_init();
  jerryxx_set_property(global, "PWM", pwm_module);
  jerry_release_value(pwm_module);
  jerry_release_value(global);
}
