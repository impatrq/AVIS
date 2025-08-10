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

#define TOLERANCIA 10 
#define REV_GPIO 14
#define FWD_GPIO 15
#define ENA 16
#define PWM_WRAP 250

QueueHandle_t queue_mpu;
QueueHandle_t queue_pitch;/////ESTA VA A SER LA COLA DE LOS DATOS DEL JUEGO

static inline void vfd_stop(void) {
    gpio_put(FWD_GPIO, false);
    gpio_put(REV_GPIO, false);
    vTaskDelay(pdMS_TO_TICKS(1));
}

static inline void vfd_forward(void) {
    gpio_put(REV_GPIO, false);
    gpio_put(FWD_GPIO, true);
    vTaskDelay(pdMS_TO_TICKS(1));
}

static inline void vfd_reverse(void) {
    gpio_put(FWD_GPIO, false);
    gpio_put(REV_GPIO, true);
    vTaskDelay(pdMS_TO_TICKS(1));
}

static inline void pwm_set_duty_percent(uint gpio, uint percent) {
    pwm_set_gpio_level(gpio, (uint16_t) (percent * PWM_WRAP / 100));
}

/*---------------------------I N I C I A L I Z A C I O N -----------------*/
void task_init(void *params) {    
    while(1) { 
    ///COLAS
    queue_mpu = xQueueCreate(1, sizeof(float));
    queue_pitch = xQueueCreate(1, sizeof(float));
    ///MOTOR
    gpio_init(FWD_GPIO);
    gpio_init(REV_GPIO);
    gpio_set_dir(FWD_GPIO, true);
    gpio_set_dir(REV_GPIO, true);
    ///MPU
    mpu6050_init();    
    ///LCD  
    lcd_init(i2c0, 0x27);
    lcd_clear();
    lcd_string("Actual: ");
    lcd_set_cursor(1, 0);
    lcd_string("Deseado: ");    
    ///PWM
    gpio_set_function( ENA, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(ENA);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, PWM_WRAP);
    pwm_config_set_clkdiv(&config, 10);
    pwm_init( 0 , &config, true);
    pwm_set_duty_percent( ENA, 5);
    vTaskDelete(NULL);    
    }
}


/*---------------------------M P U  -----------------*/
void task_mpu(void *params) { 
    while(1) {
    float pitch_actual = leer_pitch();
    xQueueOverwrite(queue_mpu, &pitch_actual);
    vTaskDelay(pdMS_TO_TICKS(500));
    }
} 

/*---------------------------L C D  -----------------*/
void task_lcd(void *params) { 
    char buffer[64];
    int index = 0;
    while(1) { 
    float pitch_actual;
    float pitch_deseado;
    xQueuePeek(queue_mpu, &pitch_actual, 500); 
    xQueuePeek(queue_pitch, &pitch_deseado, portMAX_DELAY); 
     sprintf(buffer, "%.2f", pitch_actual);
    lcd_set_cursor(0, 8);
    lcd_string(buffer);
    sprintf(buffer, "%7.2f", pitch_deseado);
    lcd_set_cursor(1, 9);
    lcd_string(buffer);
    vTaskDelay(pdMS_TO_TICKS(500));/////NO TOCAR ESTO, SINO NO SE PONE 00 EN el pitch
    }
}


/*---------------------------MOTOR -----------------*/
void task_dir(void *params){
    while(1){
    float pitch_actual;
    float pitch_deseado;
    xQueuePeek(queue_mpu, &pitch_actual, portMAX_DELAY); 
    xQueuePeek(queue_pitch, &pitch_deseado, portMAX_DELAY);
    if (pitch_actual < pitch_deseado - TOLERANCIA) {
        vfd_forward();
        vTaskDelay(pdMS_TO_TICKS(1));       
        } else if (pitch_actual > pitch_deseado + TOLERANCIA) {
        vfd_reverse();
        vTaskDelay(pdMS_TO_TICKS(1));
        } 
        else  {
        vfd_stop();    
        vTaskDelay(pdMS_TO_TICKS(1));
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
        
}

/*---------------------------L E C T U R A    D E    D A T O S   -----------------*/
void task_read(void *params) { 
    char buffer[64];
    int index = 0;
    while(1) { 
    while (!stdio_usb_connected()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    printf("✅ Pico lista para recibir datos por USB.\n");
    char buffer[128];
    while (true) {
        int pos = 0;
        while (true) {
            int ch = getchar_timeout_us(0);
            if (ch == PICO_ERROR_TIMEOUT) {
                vTaskDelay(pdMS_TO_TICKS(1));
                continue;
            }

            if (ch == '\r' || ch == '\n') {
                if (pos > 0) {
                    buffer[pos] = '\0';

                    char *ptr = strstr(buffer, "Pitch:");
                    if (ptr != NULL) {
                        char *num_start = ptr + 7;
                        while (*num_start == ' ') num_start++;
                        double rad = atof(num_start);
                        double deg = rad * (180.0 / M_PI);
                        float pitch_deseado=deg;
                        xQueueOverwrite(queue_pitch, &pitch_deseado);
                        printf("📐 Pitch convertido: %.2f grados (%.6f rad)\n", deg, rad);
                    } else {
                        printf("📝 Recibido sin Pitch: %s\n", buffer);
                    }
                    break;
                }
            } else if (pos < sizeof(buffer) - 1) {
                buffer[pos++] = (char)ch;
             }
            
            }
    }
  }
}


/*---------------------------  T A R E A S   -----------------*/
int main(void) {
    stdio_init_all();
    xTaskCreate(task_init, "INIT", configMINIMAL_STACK_SIZE,NULL, 3, NULL);
    xTaskCreate(task_mpu, "MPU", 1000,NULL, 1, NULL);
    xTaskCreate(task_lcd, "LCD", 1000,NULL, 1, NULL);
    xTaskCreate(task_dir, "MOTOR", 1000,NULL, 1, NULL);
    xTaskCreate(task_read, "PITCH", 1000 , NULL, 2, NULL);   
    vTaskStartScheduler();
    while(1);
}