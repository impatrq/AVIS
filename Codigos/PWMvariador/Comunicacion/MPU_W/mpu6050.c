#include "mpu6050.h"
#include "pico/stdlib.h"
#include <math.h>

#define MPU6050_ADDR 0x68

void mpu6050_init(i2c_inst_t *i2c) {
    uint8_t buf[] = {0x6B, 0x00};
    i2c_write_blocking(i2c, MPU6050_ADDR, buf, 2, false);
}

float leer_pitch() {
    uint8_t reg = 0x3B;
    uint8_t data[6];
    i2c_write_blocking(i2c0, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c0, MPU6050_ADDR, data, 6, false);

    int16_t acc_x = (data[0] << 8) | data[1];
    int16_t acc_z = (data[4] << 8) | data[5];

    float pitch = atan2f((float)acc_x, (float)acc_z) * 180 / M_PI;
    return pitch;
}
