#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();
    sleep_ms(2000);  // Espera a que abra el monitor serial

    int numero = 0;
    absolute_time_t proxima_impresion = make_timeout_time_ms(2000);
    sleep_ms(6000);
    char buffer[64];
    int index = 0;

    printf("Ingrese un número:\n");
    sleep_ms(1000);
    while (true) {
        // Lee carácter por carácter
        int c = getchar_timeout_us(0);
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '\r' || c == '\n') {
                buffer[index] = '\0';

                if (index > 0) {
                    numero = atoi(buffer);
                    printf("Nuevo número recibido: %d\n", numero);
                    index = 0;
                }

                printf("Ingrese un número:\n");
            } else if (index < sizeof(buffer) - 1) {
                buffer[index++] = (char)c;
            }
        }

        // Imprime el número cada 2 segundos
        if (absolute_time_diff_us(get_absolute_time(), proxima_impresion) <= 0) {
            printf("Valor actual: %d\n", numero);
            proxima_impresion = make_timeout_time_ms(2000);
        }

        sleep_ms(10);
    }

    return 0;
}
