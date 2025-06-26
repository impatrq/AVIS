
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "mpu.h"

#define ENA 16
#define PWM_WRAP 250
#define MOTOR_ON 200
#define MOTOR_OFF 230
#define TOLERANCIA 2.0

void configurar_pwm(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, PWM_WRAP);
    pwm_config_set_clkdiv(&config, 10);
    pwm_init(slice, &config, true);
    pwm_set_gpio_level(gpio, MOTOR_OFF);
  
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    configurar_pwm(ENA);
    mpu6050_init();

    char buffer[64];
    int index = 0;
    int pitch_deseado = 0;
    pitch_deseado = (int) leer_pitch();

    printf("Ingrese un ángulo de referencia:\n");

    while (true) {
        pwm_set_gpio_level(ENA, MOTOR_OFF);
        int c = getchar_timeout_us(0);

        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '\r' || c == '\n') {
                buffer[index] = '\0';
                if (index > 0) {
                    pitch_deseado = atoi(buffer);
                    printf("Ángulo deseado: %d\n", pitch_deseado);
                    index = 0;
                }
                printf("Ingrese un ángulo de referencia:\n");
            } else if (index < sizeof(buffer) - 1) {
                buffer[index++] = (char)c;
            }
        }

        double pitch_actual = leer_pitch();
        printf("Pitch actual: %.2f\n", pitch_actual);
        printf("Pitch deseado: %d\n", pitch_deseado);

        if (pitch_actual < pitch_deseado - TOLERANCIA) {
            pwm_set_gpio_level(ENA, MOTOR_ON);
        } else if (pitch_actual > pitch_deseado + TOLERANCIA) {
            pwm_set_gpio_level(ENA, MOTOR_ON);
        } else {
            pwm_set_gpio_level(ENA, MOTOR_OFF);
            printf("Ángulo alcanzado: motor detenido\n");
        }

        sleep_ms(200);
    }

    return 0;
}

