#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "lcd.h"
#include "mpu.h"
#include <math.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define TOLERANCIA 2
#define REV_GPIO 20
#define FWD_GPIO 18
#define STP_GPIO 16

QueueHandle_t queue_real_angle;
QueueHandle_t queue_game_angle;
QueueHandle_t queue_motor_real_angle;
QueueHandle_t queue_motor_game_angle;

typedef struct {
    float pitch;
    float roll;
} sensor_data_t;


typedef struct {
    float pitch;
    float roll;
} game_data_t;


/*--------------------------- I N I C I A L I Z A C I O N -----------------*/
void task_init(void *params) {   
    queue_real_angle= xQueueCreate(1, sizeof(sensor_data_t)); 
    queue_game_angle = xQueueCreate(1, sizeof(game_data_t));
    queue_motor_real_angle= xQueueCreate(1, sizeof(sensor_data_t)); 
    queue_motor_game_angle = xQueueCreate(1, sizeof(game_data_t));
    gpio_init(FWD_GPIO);
    gpio_init(REV_GPIO);
    gpio_init(STP_GPIO);
    gpio_set_dir(FWD_GPIO, true);
    gpio_set_dir(REV_GPIO, true);
    gpio_set_dir(STP_GPIO, true);
    mpu6050_init();   
    vTaskDelete(NULL);
}

/*================================== C O N S O L A  =================================*/ 
void task_consola(void *params){
    //Estructuras de angulos
    sensor_data_t angle={0};
    game_data_t game_angle={0};
    while (1) { 
        xQueueReceive(queue_real_angle,&angle,portMAX_DELAY);
        xQueueReceive(queue_game_angle,&game_angle,portMAX_DELAY);
        printf("REAL -> PITCH: %.2f°||ROLL: %.2f       GAME -> PITCH: %.2f°||ROLL: %.2f\n",angle.pitch,angle.roll,game_angle.pitch,game_angle.roll);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

/*================================ VARIADOR GTAKE ====================================*/
void task_gtake(void *params){
    sensor_data_t angle={0};
    game_data_t game_angle={0};
    while (1) { 
        xQueueReceive(queue_real_angle,&angle,portMAX_DELAY);
        xQueueReceive(queue_game_angle,&game_angle,portMAX_DELAY);
        if ( angle.pitch > game_angle.pitch - 10 && angle.pitch < game_angle.pitch + 10) {
            vfd_stop();
            vTaskDelay(pdMS_TO_TICKS(1));

        } 
        else if (angle.pitch < game_angle.pitch - TOLERANCIA) {
            vfd_forward();
            vTaskDelay(pdMS_TO_TICKS(1)); 
        } 
        else if (angle.pitch > game_angle.pitch + TOLERANCIA) {
            vfd_reverse();
            vTaskDelay(pdMS_TO_TICKS(1)); 
        }

        vTaskDelay(pdMS_TO_TICKS(50));


    }
}
        

/*==================================== VARIADOR DANFOSS ===============================*/
void task_danfoss(void *params){
    sensor_data_t angle={0};
    game_data_t game_angle={0};
    while (1) { 
        xQueueReceive(queue_real_angle,&angle,portMAX_DELAY);
        xQueueReceive(queue_game_angle,&game_angle,portMAX_DELAY);

        if ( angle.roll > game_angle.roll - 10 && angle.roll < game_angle.roll + 10) {
            vfd_stop();
            vTaskDelay(pdMS_TO_TICKS(1));

        } 
        else if (angle.roll < game_angle.roll - TOLERANCIA) {
            vfd_forward();
            vTaskDelay(pdMS_TO_TICKS(1)); 
        } 
        else if (angle.roll > game_angle.roll + TOLERANCIA) {
            vfd_reverse();
            vTaskDelay(pdMS_TO_TICKS(1)); 
        }

        vTaskDelay(pdMS_TO_TICKS(50));


    }
}


/*================================= M P U ==========================*/
void task_read(void *params) {
    sensor_data_t angle = {0};
    // Variables Kalman por eje
    static double angle_pitch = 0.0, bias_pitch = 0.0;
    static double P_pitch[2][2] = {{0,0},{0,0}};

    static double angle_roll = 0.0, bias_roll = 0.0;
    static double P_roll[2][2] = {{0,0},{0,0}};

    absolute_time_t prev_time = get_absolute_time();

    while(1) {
        
        double pitch_acc, roll_acc;
        double gyro_pitch, gyro_roll;

        leer_accel(&pitch_acc, &roll_acc);
        leer_gyro(&gyro_pitch, &gyro_roll);

        // ⿢ Calcular dt
        absolute_time_t now = get_absolute_time();
        double dt = absolute_time_diff_us(prev_time, now) / 1e6;
        prev_time = now;

        // ⿣ Aplicar filtro Kalman
        double pitch = kalman_update(pitch_acc, gyro_pitch, dt,
                                     &angle_pitch, &bias_pitch, P_pitch);
        double roll  = kalman_update(roll_acc, gyro_roll, dt,
                                     &angle_roll, &bias_roll, P_roll);
        
        angle.pitch=pitch;
        angle.roll=roll;
    
       xQueueSend(queue_real_angle,&angle,portMAX_DELAY);
      
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}


void task_game(void *params) {
    char buffer[128];
    int pos = 0;
    game_data_t game_angle={0};
    // Esperar conexión USB
    while (!stdio_usb_connected()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    printf("✅ Pico lista para recibir datos de Roll y Pitch por USB.\n");

    float rad_roll;
    float rad_pitch;

    while (1) {

        int ch = getchar_timeout_us(0);

        if (ch == PICO_ERROR_TIMEOUT) {
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        if (ch == '\r' || ch == '\n') {
            if (pos > 0) {
                buffer[pos] = '\0';  // Terminar cadena

                // --- Buscar Roll ---
                char *ptr_roll = strstr(buffer, "Roll:");
                if (ptr_roll != NULL) {
                    ptr_roll += 5;  // Avanzar después de "Bank:"
                    while (*ptr_roll == ' ') ptr_roll++;  // Saltar espacios
                    rad_roll = atof(ptr_roll);
                    float roll = rad_roll * (180 / M_PI);
                    game_angle.roll=roll;
                    
                    
                }

                // --- Buscar Pitch ---
                char *ptr_pitch = strstr(buffer, "Pitch:");
                if (ptr_pitch != NULL) {
                    ptr_pitch += 6;  // Avanzar después de "Pitch:"
                    while (*ptr_pitch == ' ') ptr_pitch++;  // Saltar espacios
                    rad_pitch = atof(ptr_pitch);
                    float pitch = rad_pitch * (180 / M_PI);
                    game_angle.pitch=pitch;    
                }
                
                xQueueSend(queue_game_angle,&game_angle,portMAX_DELAY);

                if (ptr_roll == NULL && ptr_pitch == NULL) {
                    printf("📝 Línea sin dato de Roll o Pitch: %s\n", buffer);
                }

                pos = 0;  // Reiniciar buffer para la siguiente línea
                vTaskDelay(pdMS_TO_TICKS(1));
            }
        } else if (pos < sizeof(buffer) - 1) {
            buffer[pos++] = (char)ch;  // Guardar carácter
        }
    }
}



/*---------------------------  M A I N -----------------*/
int main(void) {
    stdio_init_all();
    xTaskCreate(task_init, "INIT", configMINIMAL_STACK_SIZE, NULL, 3, NULL);   
    xTaskCreate(task_read, "READ", 1000, NULL, 2, NULL); 
    xTaskCreate(task_consola, "CONSOLA", 1000, NULL, 1, NULL); 
    xTaskCreate(task_game, "JUEGO", 1000, NULL, 2, NULL);
    xTaskCreate(task_gtake, "VARIADOR GTAKE", 1000, NULL, 1, NULL); 
    xTaskCreate(task_danfoss, "VARIADOR DANFOSS", 1000, NULL, 1, NULL);   
    vTaskStartScheduler();
    while (1);
}