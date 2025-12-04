#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "mpu.h"
#include <math.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

 
#define TOLERANCIA 7
#define REV_GPIO 19
#define FWD_GPIO 20
#define STP_GPIO 18
#define FWD_DANFOSS_GPIO 17
#define REV_DANFOSS_GPIO 16

//Varias colas para el mismo tipo de dato (pitch y roll), solo que van a ser usados por distintas tareas 
QueueHandle_t queue_real_angle;
QueueHandle_t queue_game_angle;
QueueHandle_t queue_danfoss_real_angle;
QueueHandle_t queue_danfoss_game_angle;
QueueHandle_t queue_gtake_real_angle;
QueueHandle_t queue_gtake_game_angle;

//Control del motor GTAKE(pitch)
    static inline void gtake_stop(void) {
        gpio_put(FWD_GPIO, false);
        gpio_put(REV_GPIO, false);
        gpio_put(STP_GPIO, true);
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    static inline void gtake_forward(void) {
        gpio_put(REV_GPIO, false);
        gpio_put(FWD_GPIO, true);
        gpio_put(STP_GPIO, false);
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    static inline void gtake_reverse(void) {
        gpio_put(FWD_GPIO, false);
        gpio_put(REV_GPIO, true);
        gpio_put(STP_GPIO, false);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    ///Control del motor DANFOSS(roll)
    static inline void danfoss_clkwise(void) {
        gpio_put(REV_DANFOSS_GPIO, false);
        gpio_put(FWD_DANFOSS_GPIO, true);
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    static inline void danfoss_anticlkwise(void) {
        gpio_put(FWD_DANFOSS_GPIO, false);
        gpio_put(REV_DANFOSS_GPIO, true);
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    static inline void danfoss_stop(void) {
        gpio_put(FWD_DANFOSS_GPIO, false);
        gpio_put(REV_DANFOSS_GPIO, false);
        vTaskDelay(pdMS_TO_TICKS(1));
    }

//Estructuras para mandar los tipos de datos por las colas
typedef struct {
    float pitch;
    float roll;
} sensor_data_t;


typedef struct {
    float pitch;
    float roll;
} game_data_t;


void task_init(void *params) {   
    // Varias colas con el mismo tipo de dato. Cada tarea recibe su propia copia del dato.
    //Ademas uso queuesend y queuereceive porque de esa manera evito que a las tareas le lleguen datos viejos
    queue_real_angle= xQueueCreate(1, sizeof(sensor_data_t)); 
    queue_game_angle = xQueueCreate(1, sizeof(game_data_t));
    queue_gtake_real_angle= xQueueCreate(1, sizeof(sensor_data_t)); 
    queue_gtake_game_angle = xQueueCreate(1, sizeof(game_data_t));
    queue_danfoss_real_angle= xQueueCreate(1, sizeof(sensor_data_t)); 
    queue_danfoss_game_angle = xQueueCreate(1, sizeof(game_data_t));
    gpio_init(FWD_DANFOSS_GPIO);
    gpio_init(REV_DANFOSS_GPIO);
    gpio_init(REV_GPIO);
    gpio_init(STP_GPIO);
    gpio_init(FWD_GPIO);
    gpio_set_dir(FWD_GPIO, true);
    gpio_set_dir(REV_GPIO, true);
    gpio_set_dir(STP_GPIO, true);
    gpio_set_dir(FWD_DANFOSS_GPIO, true);
    gpio_set_dir(REV_DANFOSS_GPIO, true);
    mpu6050_init();   
    vTaskDelete(NULL);
}

// Esta tarea muestra los datos angulares por Windows PowerShell   
void task_consola(void *params){
    sensor_data_t angle={0};
    game_data_t game_angle={0};
    while (1) { 
        xQueueReceive(queue_real_angle,&angle,portMAX_DELAY);
        xQueueReceive(queue_game_angle,&game_angle,portMAX_DELAY);
        printf("Roll: %7.2f GameRoll: %7.2f | Pitch: %7.2f Game Pitch: %7.2f \n",angle.roll,game_angle.roll,angle.pitch,game_angle.pitch);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}


// Esta tarea controla el motor de pitch(el variador GTAKE) 
void task_gtake(void *params){
    sensor_data_t angle={0};
    game_data_t game_angle={0};
    while (1) { 
       xQueueReceive(queue_gtake_real_angle,&angle,portMAX_DELAY);
       xQueueReceive(queue_gtake_game_angle,&game_angle,portMAX_DELAY);
       /*El motor va a estar detenido en un rango de tolerancia, fuera de ese rango se va a mover para un lado o para el otro.
        Ese rango de tolerancia esta para evitar oscilaciones del motor.*/
        if ( angle.pitch > game_angle.pitch - 7 && angle.pitch < game_angle.pitch + 7) {
            gtake_stop();
            vTaskDelay(pdMS_TO_TICKS(1));

        } 
        else if (angle.pitch < game_angle.pitch - TOLERANCIA) {
            gtake_forward();
            vTaskDelay(pdMS_TO_TICKS(1)); 
        } 
        else if (angle.pitch > game_angle.pitch + TOLERANCIA) {
            gtake_reverse();
            vTaskDelay(pdMS_TO_TICKS(1)); 
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}


// Esta tarea controla el motor de roll(el variador DANFOSS)
void task_danfoss(void *params){
    sensor_data_t angle={0};
    game_data_t game_angle={0};
    while (1) { 
        xQueueReceive(queue_danfoss_real_angle,&angle,portMAX_DELAY);
        xQueueReceive(queue_danfoss_game_angle,&game_angle,portMAX_DELAY);

        if ( angle.roll > game_angle.roll - 10 && angle.roll < game_angle.roll + 10) {
            danfoss_stop();
            vTaskDelay(pdMS_TO_TICKS(1));

        } 
        else if (angle.roll < game_angle.roll - TOLERANCIA) {
            danfoss_clkwise();
            vTaskDelay(pdMS_TO_TICKS(1)); 
        } 
        else if (angle.roll > game_angle.roll + TOLERANCIA) {
            danfoss_anticlkwise();
            vTaskDelay(pdMS_TO_TICKS(1)); 
        }

        vTaskDelay(pdMS_TO_TICKS(1));


    }
}


//Esta tarea lee los datos del sensor y le aplica el filtro de kalman
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

        // Calcular dt
        absolute_time_t now = get_absolute_time();
        double dt = absolute_time_diff_us(prev_time, now) / 1e6;
        prev_time = now;

        // Aplico filtro Kalman. Se agrego esto para evitar cambios bruscos en la medicion del sensor.
        double pitch = kalman_update(pitch_acc, gyro_pitch, dt,
                                     &angle_pitch, &bias_pitch, P_pitch);
        double roll  = kalman_update(roll_acc, gyro_roll, dt,
                                     &angle_roll, &bias_roll, P_roll);
        
        angle.pitch=pitch;
        angle.roll=roll;
    
       //Se envia el angulo real de la cabina a cada tarea que lo necesita. 
       xQueueSend(queue_real_angle,&angle,portMAX_DELAY);
       xQueueSend(queue_gtake_real_angle,&angle,portMAX_DELAY);
       xQueueSend(queue_danfoss_real_angle,&angle,portMAX_DELAY);
      
        vTaskDelay(pdMS_TO_TICKS(1)); 
    }
}


//Esta tarea extrae los datos del juego y lo envia a diferentes tareas.
void task_game(void *params) {
    char buffer[128];
    int pos = 0;
    game_data_t game_angle={0};
    // Espera conexión USB
    while (!stdio_usb_connected()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    printf("✅ Pico lista para recibir datos de Roll y Pitch por USB.\n");

    float rad_roll;
    float rad_pitch;

    while (1) {

        int ch = getchar_timeout_us(0);
        
        //Si la funcion falla por no poder tener los datos a tiempo, espera un milisegundo e intenta de nuevo. Si hay dato, sale del bucle
        if (ch == PICO_ERROR_TIMEOUT) {
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        if (ch == '\r' || ch == '\n') {
            if (pos > 0) {
                buffer[pos] = '\0';  // Termina cadena

                // Busca en el string que le llega  el "Roll:"
                char *ptr_roll = strstr(buffer, "Roll:");
                if (ptr_roll != NULL) {
                    ptr_roll += 5;  // Avanza después de "Roll:"
                    while (*ptr_roll == ' ') ptr_roll++;  // Salta espacios
                    rad_roll = atof(ptr_roll);// Convierte esos numeros ASCII a float para poder hacer operaciones
                    float roll = (rad_roll * (180 / M_PI));

                    //Limite el angulo máximo por seguridad 
                    if(roll >20){roll=20;}
                    else if(roll<-20){roll=-20;}
                    
                    game_angle.roll= -roll;//Se agrego el signo menos para que el sensor considere los angulo positivos y negativos los mismos que MSFS2020
                    
                    
                }

                // Busca Pitch 
                char *ptr_pitch = strstr(buffer, "Pitch:");
                if (ptr_pitch != NULL) {
                    ptr_pitch += 6;  // Avanza después de "Pitch:"
                    while (*ptr_pitch == ' ') ptr_pitch++;  // Salta espacios
                    rad_pitch = atof(ptr_pitch);// Convierte esos numeros ASCII a float para poder hacer operaciones
                    float pitch = (rad_pitch * (180 / M_PI))*1.5;// Se multiplica por 1.5 para poder representar mejor el angulo del avion del simulador


                    //Limite de angulo por seguridad 
                    if(pitch >20){pitch=20;}
                    else if(pitch<-20){pitch=-20;}

                    game_angle.pitch=pitch;    
                }
                

                //Se envia el angulo del avion a las tareas que necesitan el dato.   
                xQueueSend(queue_game_angle,&game_angle,portMAX_DELAY);
                xQueueSend(queue_gtake_game_angle,&game_angle,portMAX_DELAY);
                xQueueSend(queue_danfoss_game_angle,&game_angle,portMAX_DELAY);

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



/*===========================================  M A I N ======================================*/
int main(void) {
    stdio_init_all();
    xTaskCreate(task_init, "INIT", configMINIMAL_STACK_SIZE, NULL, 3, NULL);    
    xTaskCreate(task_game, "JUEGO", 1000, NULL, 2, NULL);
    xTaskCreate(task_consola, "CONSOLA", 1000, NULL, 2, NULL);
    xTaskCreate(task_read, "MPU6050", 1000, NULL, 2, NULL);
    xTaskCreate(task_gtake, "GTAKE", 1000, NULL, 2, NULL);
    xTaskCreate(task_danfoss, "DANFOSS", 1000, NULL, 2, NULL);

    vTaskStartScheduler();    while (1);
}