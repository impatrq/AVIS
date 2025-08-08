#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "mpu.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int addr = 0x68;

void mpu6050_init() {
    uint8_t buf[] = {0x6B, 0x80};
    i2c_init(i2c_default, 100000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    i2c_write_blocking(i2c_default, addr, buf, 2, false);
    sleep_ms(100);
    buf[1] = 0x00;
    i2c_write_blocking(i2c_default, addr, buf, 2, false);
    sleep_ms(10);
}

double leer_pitch() {
    uint8_t buffer[6];
    int16_t accel[3];

    uint8_t val = 0x3B;
    i2c_write_blocking(i2c_default, addr, &val, 1, true);
    i2c_read_blocking(i2c_default, addr, buffer, 6, false);

    for (int i = 0; i < 3; i++) {
        accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
    }

    double acc1 = accel[0] * (9.81 / 16384.0);
    double acc2 = accel[1] * (9.81 / 16384.0);
    double acc3 = accel[2] * (9.81 / 16384.0);

    double pitch = atan2(-acc1, sqrt(acc2 * acc2 + acc3 * acc3)) * 180.0 / M_PI;

    return pitch;
}
