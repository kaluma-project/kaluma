const rp2_native = process.binding(process.binding.cyw43_arch);

const CYW43_ARCH_WL_GPIO_LED_PIN = 0

function gpioPut(pin, value) {
  rp2_native.gpioPut(pin, value);
}

exports.CYW43_ARCH_WL_GPIO_LED_PIN = CYW43_ARCH_WL_GPIO_LED_PIN;
exports.gpioPut = gpioPut;
