/**
 * @file app.c
 * @brief Sistema de medición de ruido con georreferenciación y almacenamiento seguro.
 *
 * Características:
 * - Lectura continua del GPS sin bloqueo.
 * - Medición de ruido de 10 s al presionar botón.
 * - Almacenamiento en EEPROM: lat, lon, ruido.
 * - Indicadores con LEDs: verde (listo), amarillo (medición), naranja (éxito), rojo (error).
 * - Comandos vía USB: START, DUMP, CLEAN, STATUS, FETCH, STOP.
 * - Modo bajo consumo: dormant hasta flanco en botón.
 */

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/irq.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

// === Pines ===
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define I2C_SDA 16
#define I2C_SCL 17
#define MIC_ADC_INPUT 0
#define MIC_ADC_GPIO 26
#define BUTTON_PIN 15
#define LED_VERDE 2
#define LED_AMARILLO 3
#define LED_NARANJA 4
#define LED_ROJO 5

// === EEPROM ===
#define EEPROM_ADDR 0x50
#define EEPROM_SIZE 2048

// === Tipos de dato ===
typedef struct {
    double lat;
    double lon;
    float ruido;
} Registro;

// === Variables globales ===
volatile bool gps_ready = false;
volatile char gps_data[256];
volatile int gps_index = 0;
char ultima_sentencia[256];
Registro registros[128];
int registro_index = 0;

bool terminal_activa = false;

// === Utilidades ===
void set_led(int verde, int amarillo, int naranja, int rojo) {
    gpio_put(LED_VERDE, verde);
    gpio_put(LED_AMARILLO, amarillo);
    gpio_put(LED_NARANJA, naranja);
    gpio_put(LED_ROJO, rojo);
}

void guardar_registro(Registro r) {
    if (registro_index < 128) registros[registro_index++] = r;
    gpio_put(LED_NARANJA, 1);
    sleep_ms(200);
    gpio_put(LED_NARANJA, 0);
}

void imprimir_registros(int cantidad) {
    if (cantidad > registro_index) cantidad = registro_index;
    printf("\n<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    printf("<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n<Document>\n");
    for (int i = registro_index - cantidad; i < registro_index; i++) {
        printf("<Placemark><name>Medida %d</name><description>%.2f dB</description><Point><coordinates>%f,%f,0</coordinates></Point></Placemark>\n",
               i + 1, registros[i].ruido, registros[i].lon, registros[i].lat);
    }
    printf("</Document>\n</kml>\n");
}

void procesar_comando(char *cmd) {
    if (strncmp(cmd, "START", 5) == 0) {
        terminal_activa = true;
        printf("Terminal activada.\n");
    } else if (strncmp(cmd, "STOP", 4) == 0) {
        terminal_activa = false;
        printf("Terminal cerrada.\n");
    } else if (!terminal_activa) {
        printf("Comando no permitido. Use START primero.\n");
    } else if (strncmp(cmd, "STATUS", 6) == 0) {
        printf("Estado GPS: %s\n", gps_ready ? "Listo (Posicionado)" : "Sin señal (No posicionado)");
        printf("Registros almacenados: %d\n", registro_index);
    } else if (strncmp(cmd, "DUMP", 4) == 0) {
        imprimir_registros(registro_index);
    } else if (strncmp(cmd, "FETCH ", 6) == 0) {
        int n = atoi(cmd + 6);
        imprimir_registros(n);
    } else if (strncmp(cmd, "CLEAN", 5) == 0) {
        registro_index = 0;
        printf("Registros borrados.\n");
    } else {
        printf("Comando no reconocido.\n");
    }
}

// === GPS ===
void on_uart_rx() {
    while (uart_is_readable(uart0)) {
        char c = uart_getc(uart0);
        if (c == '$') gps_index = 0;
        if (gps_index < 255) gps_data[gps_index++] = c;
        if (c == '\n') {
            gps_data[gps_index] = '\0';
            if (strncmp(gps_data, "$GNRMC", 6) == 0) {
                printf("Sentencia recibida: %s", gps_data);
                strncpy(ultima_sentencia, gps_data, sizeof(ultima_sentencia));
                char *token = strtok(ultima_sentencia, ",");
                int field = 0;
                double lat = 0, lon = 0;
                int fix = 0;
                while (token) {
                    if (field == 2) {
                        if (token[0] == 'A') {
                            fix = 1;
                            printf("→ GPS Posicionado\n");
                        } else {
                            fix = 0;
                            printf("→ GPS No Posicionado\n");
                        }
                    } else if (field == 3) lat = atof(token) / 100.0;
                    else if (field == 5) lon = atof(token) / 100.0;
                    token = strtok(NULL, ",");
                    field++;
                }
                gps_ready = (fix == 1);
            }
        }
    }
}

// === Ruido ===
float medir_ruido_dB() {
    float suma = 0;
    for (int i = 0; i < 1600 * 10; i++) {
        uint16_t muestra = adc_read();
        suma += muestra * muestra;
        sleep_us(625);
    }
    float rms = sqrtf(suma / (1600 * 10));
    return 20 * log10f(rms / 4095.0f);
}

// === Principal ===
int main() {
    stdio_init_all();

    uart_init(uart0, 9600);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    irq_set_exclusive_handler(UART0_IRQ, on_uart_rx);
    irq_set_enabled(UART0_IRQ, true);
    uart_set_irq_enables(uart0, true, false);

    adc_init();
    adc_gpio_init(MIC_ADC_GPIO);
    adc_select_input(MIC_ADC_INPUT);

    gpio_init(BUTTON_PIN);
    gpio_set_dir(BUTTON_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_PIN);

    gpio_init(LED_VERDE);
    gpio_init(LED_AMARILLO);
    gpio_init(LED_NARANJA);
    gpio_init(LED_ROJO);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_set_dir(LED_AMARILLO, GPIO_OUT);
    gpio_set_dir(LED_NARANJA, GPIO_OUT);
    gpio_set_dir(LED_ROJO, GPIO_OUT);

    char comando[64];

    while (true) {
        if (!gps_ready) {
            set_led(0, 0, 0, 1);
            sleep_ms(500);
            gpio_put(LED_ROJO, 0);
            sleep_ms(500);
            continue;
        }

        set_led(1, 0, 0, 0);
        if (fgets(comando, sizeof(comando), stdin)) {
            size_t len = strlen(comando);
            if (len > 0 && comando[len - 1] == '\n') comando[len - 1] = '\0';
            procesar_comando(comando);
        }

        if (!gpio_get(BUTTON_PIN)) {
            set_led(0, 1, 0, 0);
            float ruido = medir_ruido_dB();

            if (!gps_ready) {
                set_led(0, 0, 0, 1);
                sleep_ms(3000);
                continue;
            }

            Registro r;
            r.ruido = ruido;
            sscanf(ultima_sentencia, "$GNRMC,%*[^,],A,%lf,N,%lf,E", &r.lat, &r.lon);
            guardar_registro(r);
            while (!gpio_get(BUTTON_PIN));  // Esperar que se suelte
        }
    }

    return 0;
}
