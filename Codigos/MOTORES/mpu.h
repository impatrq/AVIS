#ifndef MPU_H
#define MPU_H

// Inicialización del MPU6050
void mpu6050_init(void);

// Lecturas del acelerómetro (ángulo absoluto)
void leer_accel(double *pitch, double *roll);

// Lectura del giroscopio (velocidad angular)
void leer_gyro(double *gyro_pitch, double *gyro_roll);

// Filtro de Kalman 
double kalman_update(double newAngle, double newRate, double dt,
                     double *angle, double *bias, double P[2][2]);

#endif
