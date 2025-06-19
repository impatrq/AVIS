#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/i2c.h"
#include "mpu6050.h"

#define I2C_PORT i2c0
#define SDA_PIN 4
#define SCL_PIN 5

#define UDP_PORT 1234
#define DEST_IP  "192.168.4.1"  // IP de la otra Pico

int main() {
    stdio_init_all();
    if (cyw43_arch_init()) {
        printf("Fallo init Wi-Fi\n");
        return -1;
    }

    cyw43_arch_enable_sta_mode();
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    // Configurar I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(1000);
    mpu6050_init();

    // Crear socket UDP
    int sock = lwip_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0) {
        printf("Error al crear socket\n");
        return -1;
    }

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = PP_HTONS(UDP_PORT);
    dest_addr.sin_addr.s_addr = ipaddr_addr(DEST_IP);

    while (true) {
        float pitch = leer_pitch();
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.2f", pitch);
        lwip_sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
        sleep_ms(200); // 5 Hz
    }

    return 0;
}
