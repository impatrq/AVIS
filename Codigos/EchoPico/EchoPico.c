#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"

int main() {
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("✅ Pico lista para recibir datos por USB.\n");

    char buffer[128];
    while (true) {
        int pos = 0;

        while (true) {
            int ch = getchar_timeout_us(0);
            if (ch == PICO_ERROR_TIMEOUT) {
                sleep_ms(1);
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

    return 0;
}
