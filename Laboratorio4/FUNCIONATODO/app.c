/**
 * @file app.c
 * @brief Programa principal que integra operaciones con I2C, temporizadores, UART y GPS para almacenar y recuperar datos en EEPROM.
 * 
 * @authors
 * - Camilo Andres Anacona Anacona
 * - Maria Valentina Quiroga Alzate
 */

#include "eeprom.h"           ///< Funciones para manejo de memoria EEPROM vía I2C
#include "timer.h"            ///< Módulo para temporizadores y alarmas
#include "gps.h"              ///< Funciones para manejo de datos GPS
#include <stdlib.h>           ///< Biblioteca estándar de C
#include "ruido.h"            ///< Funciones para medición de nivel de ruido
#include "pico/stdlib.h"      ///< Funciones estándar para Raspberry Pi Pico
#include "pico/multicore.h"   ///< Para manejo de múltiples núcleos
#include <math.h>             ///< Funciones matemáticas

/** @brief Pin del LED integrado */
#define LED_PIN 25

/** @brief Pin para LED verde (estado activo) */
#define LED_VERDE 3

/** @brief Pin para LED rojo (estado GPS inválido) */
#define LED_ROJO 5

/** @brief Pin para LED amarillo (medición de ruido) */
#define LED_AMARILLO 4

/** @brief Pin para LED naranja (fin de captura) */
#define LED_NARANJA 8

/** @brief Pin del botón físico que activa la medición de ruido */
#define BOTON_OK 15

bool prev_boton = true;                      ///< Estado anterior del botón
bool bstart = false, bstop = false, bstatus = false, bclean = false;  ///< Flags para comandos UART
volatile bool bestado = false;               ///< Estado del GPS (posicionado o no)
volatile bool benableW = false;              ///< Habilita escritura si hay posición válida
int tiempo = 0;                              ///< Tiempo acumulado (no utilizado directamente)
volatile bool medir_ruido_flag = false;      ///< Bandera para iniciar medición de ruido

uint16_t writeAddr = 0, posAddr = 0;         ///< Direcciones de escritura en EEPROM
uint16_t currentAddress = 0;                 ///< Dirección actual en EEPROM

int n = 0;                                   ///< Cantidad de registros a leer (FETCH)

char buffer[BUFFSIZE];                       ///< Búfer para frases NMEA del GPS
volatile int buffer_index = 0;               ///< Índice del búfer de GPS

/**
 * @brief Función ejecutada en el segundo núcleo. Lee comandos desde consola UART.
 * 
 * Comandos reconocidos:
 * - START: habilita el sistema
 * - OK: inicia medición de sonido
 * - STATUS: muestra estado GPS y conteo de datos
 * - FETCH n: recupera n registros desde EEPROM
 * - CLEAN: limpia memoria
 * - STOP: detiene el sistema
 */
void core1_process(){
    char data[11];  ///< Búfer de entrada para comandos
    while (1)
    {
        if (fgets(data, sizeof(data), stdin) != NULL) {
            size_t len = strlen(data);
            if (len > 0 && data[len - 1] == '\n') data[len - 1] = '\0';
            printf("---- Sent utf8 encoded message: \"%s\\n\" ----\n", data);

            if(strncmp(data, "START", 5) == 0) {
                printf("Terminal disponible.\n");
                bstart = true;
                gpio_put(LED_VERDE, 1);
            }
            else if (bstart && strncmp(data, "OK", 2) == 0) {
                printf("Iniciando captura de sonido por 10 segundos...\n");
                medir_ruido_flag = true;
            }
            else if (bstart && strncmp(data, "STATUS", 6) == 0) {
                printf("Estado del GPS: %s\n", (bestado) ? "Posicionado" : "No posicionado");
                if (currentAddress != 0) {
                    printf("Num datos: %d\n", ((currentAddress - sizeof(struct Coordenadas)) / sizeof(struct Coordenadas)));
                    printf("Num registro: %d\n", (currentAddress - sizeof(struct Coordenadas)));
                } else {
                    printf("Num datos: 0\n");
                    printf("Num registros: 0\n");
                }
                if (bestado)
                    printf("Tiempo GPS activo: %02d:%02d:%02d\n", elapsed_hours, elapsed_minutes, elapsed_seconds);
                else
                    printf("Tiempo GPS activo: 00:00:00\n");
                bstatus = true;
                bclean = false;
            } else if (bstart && strncmp(data, "FETCH ", 6) == 0) {
                n = atoi(data + 6);
                bstatus = false;
                bclean = false;
            } else if (bstart && strncmp(data, "CLEAN", 5) == 0){
                currentAddress = 0;
                printf("Clean Eeprom.\n");
                printf("Posicion: %d\n", currentAddress);
                bclean = true;
                bstatus = false;
            } else if (bstart && strncmp(data, "STOP", 4) == 0){
                printf("Terminal cerrada.\n");
                gpio_put(LED_VERDE, 0);
                bstart = false;
                bstatus = false;
                bclean = false;
            } else {
                printf("Comando no reconocido.\n");
            }
        }
    }
}

/**
 * @brief Función principal del programa.
 * 
 * Inicializa periféricos, configura interrupciones y ejecuta ciclo principal que:
 * - Procesa datos GPS
 * - Gestiona escritura en EEPROM
 * - Controla LEDs de estado
 * - Inicia medición de ruido desde consola o botón
 */
int main() {
    stdio_init_all(); ///< Inicializa USB serial
    while (!stdio_usb_connected()) {
        sleep_ms(10);
    }

    configure_i2c();                   ///< Inicializa bus I2C
    multicore_launch_core1(core1_process); ///< Lanza función de comandos en core 1
    init_timer();                     ///< Inicializa temporizador
    configurePinUart();               ///< Configura pines UART
    configIntUart();                  ///< Configura interrupciones UART

    // Configura LEDs
    gpio_init(LED_PIN); gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_init(LED_VERDE); gpio_set_dir(LED_VERDE, GPIO_OUT); gpio_put(LED_VERDE, 0);
    gpio_init(LED_ROJO); gpio_set_dir(LED_ROJO, GPIO_OUT); gpio_put(LED_ROJO, 0);
    gpio_init(LED_AMARILLO); gpio_set_dir(LED_AMARILLO, GPIO_OUT); gpio_put(LED_AMARILLO, 0);
    gpio_init(LED_NARANJA); gpio_set_dir(LED_NARANJA, GPIO_OUT); gpio_put(LED_NARANJA, 0);

    // Configura botón físico
    gpio_init(BOTON_OK);
    gpio_set_dir(BOTON_OK, GPIO_IN);
    gpio_pull_up(BOTON_OK);

    init_adc_microfono(); ///< Inicializa canal ADC para micrófono

    printf("Tamaño struct Coordenadas: %d bytes\n", sizeof(struct Coordenadas));

    // --- Bucle principal ---
    while (1) {

        // Procesamiento de datos GPS por UART
        if (uart_flag) {
            uart_flag = false;
            if (data == '$') buffer_index = 0;
            buffer[buffer_index++] = data;

            if (data == '\n') {
                buffer[buffer_index] = '\0';
                process_nmea_sentence(buffer);

                if (GPS.Status == 1) {
                    gpio_put(LED_ROJO, 0);
                    gpio_put(LED_PIN, 1);
                    bestado = true;
                    benableW = true;
                } else {
                    gpio_put(LED_ROJO, 1);
                    gpio_put(LED_PIN, 0);
                    bestado = false;
                    shora = 0; sminuto = 0; ssegundo = 0;
                }
            }
        } else {
            benableW = false;
        }

        // Detección de flanco descendente del botón
        bool boton_actual = gpio_get(BOTON_OK);
        if (prev_boton && !boton_actual && bstart) {
            printf("Botón presionado: iniciando captura de sonido por 10 segundos...\n");
            medir_ruido_flag = true;
        }
        prev_boton = boton_actual;

        // Escritura automática cada ciclo de alarma
        if (benableW && alarm_fired_1 && GPS.Status == 1) {
            if (!isnan(GPS.Lat) && !isnan(GPS.Lon) &&
                GPS.Lat >= -90.0 && GPS.Lat <= 90.0 &&
                GPS.Lon >= -180.0 && GPS.Lon <= 180.0) {

                struct Coordenadas datos = {GPS.Lat, GPS.Lon, 0.0f};
                writeEeprom(currentAddress, datos);
                printf("→ Posición guardada: %.6f, %.6f @ Addr=%d\n", GPS.Lat, GPS.Lon, currentAddress);
                currentAddress += sizeof(struct Coordenadas);
                if (currentAddress >= 2048) currentAddress = 0;
            }

            if (n != 0) {
                printf("Leyendo desde la EEPROM...\n");
                nDataEeprom(n, currentAddress - sizeof(struct Coordenadas));
                n = 0;
            }

            alarm_fired_1 = false;
        }

        // Captura de sonido
        if (medir_ruido_flag) {
            medir_ruido_flag = false;
            printf("Midiendo nivel de ruido...\n");
            gpio_put(LED_VERDE, 0);
            gpio_put(LED_AMARILLO, 1);

            for (int i = 0; i < 20; i++) {
                float ruido = medir_ruido_dB_rapido();
                printf("Muestra %d: %.2f dB\n", i + 1, ruido);

                if (GPS.Status == 1 &&
                    !isnan(GPS.Lat) && !isnan(GPS.Lon) &&
                    GPS.Lat >= -90.0 && GPS.Lat <= 90.0 &&
                    GPS.Lon >= -180.0 && GPS.Lon <= 180.0) {

                    struct Coordenadas datos = {GPS.Lat, GPS.Lon, ruido};
                    writeEeprom(currentAddress, datos);
                    printf("EEPROM Write: %.2f dB @ %.6f, %.6f\n", datos.dB, datos.dato1, datos.dato2);
                    printf("→ Escrito en EEPROM: %.2f dB @ lat=%.6f, lon=%.6f | Addr=%d\n", ruido, GPS.Lat, GPS.Lon, currentAddress);
                    currentAddress += sizeof(struct Coordenadas);
                    if (currentAddress >= 2048) currentAddress = 0;
                } else {
                    printf("No hay señal GPS válida. Muestra descartada.\n");
                }

                sleep_ms(500);
            }

            gpio_put(LED_AMARILLO, 0);
            gpio_put(LED_NARANJA, 1);
            sleep_ms(2000);
            gpio_put(LED_NARANJA, 0);
            gpio_put(LED_VERDE, 1);

            printf("Captura finalizada.\n");
        }
    }

    return 0;
}
