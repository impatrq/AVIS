#include "mpu.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int addr = 0x68;

// ====== Variables Kalman internas ======
static double angle_pitch = 0.0, bias_pitch = 0.0;
static double P_pitch[2][2] = {{0,0},{0,0}};

static double angle_roll = 0.0, bias_roll = 0.0;
static double P_roll[2][2] = {{0,0},{0,0}};

// Constantes 
const double Q_angle = 0.001;
const double Q_bias = 0.003;
const double R_measure = 0.03;

// ====== Inicialización MPU6050 ======
void mpu6050_init(void) {
    uint8_t buf[2] = {0x6B, 0x80};
    i2c_init(i2c1, 100000);
    gpio_set_function(27, GPIO_FUNC_I2C);
    gpio_set_function(26, GPIO_FUNC_I2C);
    gpio_pull_up(27);
    gpio_pull_up(26);

    i2c_write_blocking(i2c1, addr, buf, 2, false);
    sleep_ms(100);
    buf[1] = 0x00;
    i2c_write_blocking(i2c1, addr, buf, 2, false);
    sleep_ms(10);
}

// ====== Lectura del acelerómetro ======
void leer_accel(double *pitch, double *roll) {
    uint8_t buffer[6];
    int16_t accel[3];

    uint8_t val = 0x3B;
    i2c_write_blocking(i2c1, addr, &val, 1, true);
    i2c_read_blocking(i2c1, addr, buffer, 6, false);

    for(int i=0; i<3; i++)
        accel[i] = (buffer[i*2] << 8) | buffer[i*2 + 1];

    double ax = accel[0] / 16384.0;
    double ay = accel[1] / 16384.0;
    double az = accel[2] / 16384.0;

    *pitch = atan2(-ax, sqrt(ay*ay + az*az)) * 180.0 / M_PI;
    *roll  = atan2(ay, az) * 180.0 / M_PI;
}

// ====== Lectura del giroscopio ======
void leer_gyro(double *gyro_pitch, double *gyro_roll) {
    uint8_t buffer[6];
    int16_t gyro_raw[3];

    uint8_t val = 0x43;
    i2c_write_blocking(i2c1, addr, &val, 1, true);
    i2c_read_blocking(i2c1, addr, buffer, 6, false);

    for(int i=0; i<3; i++)
        gyro_raw[i] = (buffer[i*2] << 8) | buffer[i*2 + 1];

    *gyro_roll  = gyro_raw[0] / 131.0; // eje X → roll
    *gyro_pitch = gyro_raw[1] / 131.0; // eje Y → pitch
}


double kalman_update(double newAngle, double newRate, double dt,
                     double *angle, double *bias, double P[2][2]) {

    double rate = newRate - *bias;
    *angle += dt * rate;

    P[0][0] += dt * (dt*P[1][1] - P[0][1] - P[1][0] + Q_angle);
    P[0][1] -= dt * P[1][1];
    P[1][0] -= dt * P[1][1];
    P[1][1] += Q_bias * dt;

    double S = P[0][0] + R_measure;
    double K[2];
    K[0] = P[0][0] / S;
    K[1] = P[1][0] / S;

    double y = newAngle - *angle;
    *angle += K[0] * y;
    *bias  += K[1] * y;

    double P00_temp = P[0][0];
    double P01_temp = P[0][1];

    P[0][0] -= K[0] * P00_temp;
    P[0][1] -= K[0] * P01_temp;
    P[1][0] -= K[1] * P00_temp;
    P[1][1] -= K[1] * P01_temp;

    return *angle;
}
