#ifndef MPU6050_H
#define MPU6050_H

#include "hardware/i2c.h"

void mpu6050_init(i2c_inst_t *i2c);
float leer_pitch();

#endif
