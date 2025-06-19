#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/pwm.h"
#include "lwip/udp.h"
#include "lwip/ip_addr.h"

#define WIFI_SSID "TU_SSID"
#define WIFI_PASS "TU_PASSWORD"
#define PWM_GPIO 15
#define PWM_WRAP 12500

float received_angle = 0;

void udp_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                 const ip_addr_t *addr, u16_t port) {
    if (p != NULL) {
        char buf[32] = {0};
        memcpy(buf, p->payload, p->len);
        received_angle = atof(buf);
        printf("Ángulo recibido: %.2f\n", received_angle);
        pbuf_free(p);
    }
}

void init_pwm(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice, PWM_WRAP);
    pwm_set_chan_level(slice, PWM_CHAN_A, 0);
    pwm_set_enabled(slice, true);
}

void set_pwm(uint gpio, bool on) {
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_chan_level(slice, PWM_CHAN_A, on ? 125 : 0);
}

int main() {
    stdio_init_all();
    init_pwm(PWM_GPIO);

    if (cyw43_arch_init()) return 1;
    cyw43_arch_enable_sta_mode();

    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS,
        CYW43_AUTH_WPA2_AES_PSK, 10000)) {
        printf("Wi-Fi fallo\n");
        return 1;
    }

    printf("Wi-Fi conectado\n");

    struct udp_pcb *pcb = udp_new();
    if (!pcb) {
        printf("Error creando UDP\n");
        return 1;
    }

    udp_bind(pcb, IP_ADDR_ANY, 1234);
    udp_recv(pcb, udp_recv_cb, NULL);

    float objetivo = 0;
    char input[16];

    while (true) {
        if (fgets(input, sizeof(input), stdin)) {
            objetivo = atof(input);
            printf("Ángulo objetivo: %.2f\n", objetivo);
        }

        if (received_angle > objetivo + 1.0f)
            set_pwm(PWM_GPIO, true);
        else if (received_angle < objetivo - 1.0f)
            set_pwm(PWM_GPIO, true);
        else
            set_pwm(PWM_GPIO, false);

        sleep_ms(100);
    }a
}
