#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define ENA 16 // Pin conectado a ENA del L298N

void configurar_pwm(uint gpio, uint16_t duty_cycle, uint16_t wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    uint canal = pwm_gpio_to_channel(gpio);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, wrap);
    pwm_config_set_clkdiv(&config, 10);
    pwm_config_set_output_polarity(&config, true, true);

    pwm_init(slice, &config, true); // Inicia el PWM con la config
//    pwm_set_chan_level(slice, canal, duty_cycle); // Ciclo útil inicial
    pwm_set_gpio_level(gpio, duty_cycle);
}

int main() {
    set_sys_clock_khz(125000, true);
    stdio_init_all();
    const uint16_t wrap = 250;        
    configurar_pwm(ENA, 0, wrap);

    while (true) {

//        pwm_set_chan_level(pwm_gpio_to_slice_num(ENA), pwm_gpio_to_channel(ENA), 150);
        pwm_set_gpio_level(ENA, 125);
        sleep_ms(5000);
    }
}