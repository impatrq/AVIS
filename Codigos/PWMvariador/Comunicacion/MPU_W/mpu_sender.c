#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "mpu6050.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#define I2C_PORT i2c0
#define SDA_PIN 4
#define SCL_PIN 5
#define WIFI_SSID "TU_SSID"
#define WIFI_PASS "TU_PASSWORD"
#define DEST_IP "192.168.0.100"  // IP de la Pico que recibe
#define DEST_PORT 1234

struct udp_pcb *udp_client;

void send_pitch(float pitch) {
    char msg[32];
    snprintf(msg, sizeof(msg), "%.2f", pitch);

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, strlen(msg), PBUF_RAM);
    if (!p) return;

    memcpy(p->payload, msg, strlen(msg));
    ip_addr_t dest_ip;
    ipaddr_aton(DEST_IP, &dest_ip);
    udp_sendto(udp_client, p, &dest_ip, DEST_PORT);
    pbuf_free(p);
}

int main() {
    stdio_init_all();
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    mpu6050_init(I2C_PORT);

    if (cyw43_arch_init()) {
        printf("Wi-Fi init failed\n");
        return 1;
    }

    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS, CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Wi-Fi connection failed\n");
        return 1;
    }

    printf("Wi-Fi connected\n");

    udp_client = udp_new();
    if (!udp_client) {
        printf("Failed to create UDP client\n");
        return 1;
    }

    while (true) {
        float pitch = leer_pitch();
        printf("Pitch: %.2f\n", pitch);
        send_pitch(pitch);
        sleep_ms(200);
    }
}
