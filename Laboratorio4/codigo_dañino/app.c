#include "eeprom.h"
#include "timer.h"
#include "gps.h"
#include "ruido.h"
#include "pico/multicore.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/uart.h"
#include <stdlib.h>

#define LED_PIN         25
#define LED_VERDE       2
#define LED_AMARILLO    3
#define LED_NARANJA     4
#define LED_ROJO        5
#define PULSADOR_PIN    15

volatile bool bestado = false;
volatile uint16_t currentAddress = 0;
volatile char data;

bool bstart = false, bstatus = false, bclean = false;
int n = 0;
char buffer[BUFFSIZE];
volatile int buffer_index = 0;
volatile bool midiendo = false;

// ────── UART ISR para GPS ──────
void on_uart_rx() {
    while (uart_is_readable(uart0)) {
        data = uart_getc(uart0);
        uart_flag = true;
    }
}

// ────── Core 1: Terminal y botón ──────
void core1_process() {
    char comando[16];

    gpio_init(PULSADOR_PIN); gpio_set_dir(PULSADOR_PIN, GPIO_IN); gpio_pull_up(PULSADOR_PIN);
    gpio_init(LED_VERDE); gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_init(LED_AMARILLO); gpio_set_dir(LED_AMARILLO, GPIO_OUT);
    gpio_init(LED_NARANJA); gpio_set_dir(LED_NARANJA, GPIO_OUT);
    gpio_init(LED_ROJO); gpio_set_dir(LED_ROJO, GPIO_OUT);
    gpio_put(LED_VERDE, 1);

    while (1) {
        // ─ Terminal serial ─
        if (fgets(comando, sizeof(comando), stdin) != NULL) {
            size_t len = strlen(comando);
            if (len > 0 && comando[len - 1] == '\n') comando[len - 1] = '\0';

            if (strncmp(comando, "START", 5) == 0) {
                printf("Terminal disponible.\n");
                bstart = true;
            } else if (bstart && strncmp(comando, "STATUS", 6) == 0) {
                printf("Estado del GPS: %s\n", (bestado ? "Posicionado" : "No posicionado"));
                printf("Num datos: %d\n", (currentAddress ? (currentAddress / sizeof(struct Coordenadas)) : 0));
                printf("Num registro: %d\n", currentAddress);
                if (bestado)
                    printf("Tiempo GPS activo: %02d:%02d:%02d\n", elapsed_hours, elapsed_minutes, elapsed_seconds);
                else
                    printf("Tiempo GPS activo: 00:00:00\n");
            } else if (bstart && strncmp(comando, "FETCH ", 6) == 0) {
                n = atoi(comando + 6);
            } else if (bstart && strncmp(comando, "CLEAN", 5) == 0) {
                currentAddress = 0;
                printf("Clean EEPROM.\n");
            } else if (bstart && strncmp(comando, "STOP", 4) == 0) {
                printf("Terminal cerrada.\n");
                bstart = false;
            } else {
                printf("Comando no reconocido.\n");
            }
        }

        // ─ Lógica del botón ─
        if (gpio_get(PULSADOR_PIN) == 0 && bestado && !midiendo) {
            sleep_ms(50);
            if (gpio_get(PULSADOR_PIN) == 0) {
                midiendo = true;
                gpio_put(LED_VERDE, 0);
                gpio_put(LED_AMARILLO, 1);

                float ruido = medir_ruido_dB();

                if (!bestado) {
                    gpio_put(LED_AMARILLO, 0);
                    gpio_put(LED_ROJO, 1);
                    sleep_ms(2000);
                    gpio_put(LED_ROJO, 0);
                    gpio_put(LED_VERDE, 1);
                    midiendo = false;
                    continue;
                }

                struct Coordenadas coord = { GPS.Lat, GPS.Lon, ruido };
                writeEeprom(currentAddress, coord);
                currentAddress += sizeof(struct Coordenadas);
                if (currentAddress >= 2048) currentAddress = 0;

                gpio_put(LED_AMARILLO, 0);
                gpio_put(LED_NARANJA, 1);
                sleep_ms(500);
                gpio_put(LED_NARANJA, 0);
                gpio_put(LED_VERDE, 1);
                midiendo = false;
            }

            while (gpio_get(PULSADOR_PIN) == 0) tight_loop_contents();  // Esperar liberación
        }

        // ─ FETCH EEPROM ─
        if (n != 0) {
            printf("Leyendo desde la EEPROM...\n");
            nDataEeprom(n, currentAddress - sizeof(struct Coordenadas));
            n = 0;
        }
    }
}

// ────── Main: GPS en core0 ──────
int main() {
    stdio_init_all();
    configure_i2c();
    init_timer();
    configurePinUart();
    configIntUart();
    init_adc_microfono();

    gpio_init(LED_PIN); gpio_set_dir(LED_PIN, GPIO_OUT);
    uart_set_irq_enables(uart0, true, false);
    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);

    multicore_launch_core1(core1_process);

    while (1) {
        if (uart_flag) {
            uart_flag = false;

            if (data == '$') buffer_index = 0;
            buffer[buffer_index++] = data;

            if (data == '\n') {
                buffer[buffer_index] = '\0';
                process_nmea_sentence(buffer);

                if (GPS.Status == 1) {
                    gpio_put(LED_PIN, 1);
                    bestado = true;
                } else {
                    gpio_put(LED_PIN, 0);
                    bestado = false;
                    shora = sminuto = ssegundo = 0;
                }
            }
        }
    }
    return 0;
}
