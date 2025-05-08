#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    // Esperar hasta que USB esté listo
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("✅ Pico lista para recibir datos por USB.\n");

    char buffer[128];
    while (true) {
        int pos = 0;

        // Leer hasta nueva línea
        while (true) {
            int ch = getchar_timeout_us(0);
            if (ch == PICO_ERROR_TIMEOUT) {
                sleep_ms(1);
                continue;
            }

            if (ch == '\r' || ch == '\n') {
                if (pos > 0) {
                    buffer[pos] = '\0';
                    printf("PICO:%s\n", buffer);  // Eco a PowerShell
                    break;
                }
            } else if (pos < sizeof(buffer) - 1) {
                buffer[pos++] = (char)ch;
            }
        }
    }

    return 0;
}