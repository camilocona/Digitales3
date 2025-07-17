/**
 * @file gps.h
 * @brief Programa para la gestión de datos NMEA de un módulo GPS utilizando la UART.
 *
 * Este módulo permite:
 * - Configurar la UART para recepción de datos GPS.
 * - Capturar caracteres recibidos por interrupción.
 * - Procesar sentencias NMEA tipo $GNRMC para extraer latitud, longitud y hora.
 * - Calcular el tiempo transcurrido desde que el GPS obtiene señal válida.
 *
 * @authors
 * - Camilo Andres Anacona Anacona
 * - Maria Valentina Quiroga Alzate
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include <string.h>

/** @brief Identificador de la UART utilizada para el GPS */
#define UART_ID uart0

/** @brief Velocidad de transmisión de la UART */
#define BAUD_RATE 9600

/** @brief Tamaño máximo del búfer de datos NMEA */
#define BUFFSIZE 256

/** @brief Pin de transmisión UART */
#define UART_TX_PIN 0

/** @brief Pin de recepción UART */
#define UART_RX_PIN 1

/** @brief Bandera que indica si se recibió un carácter por UART */
volatile bool uart_flag = false;

/** @brief Carácter recibido por la UART */
volatile char data;

/** 
 * @brief Variables para registrar la hora en que el GPS obtuvo señal válida por primera vez
 */
volatile int shora = 0, sminuto = 0, ssegundo = 0;

/**
 * @brief Tiempo transcurrido desde la primera posición válida
 */
volatile int elapsed_hours = 0, elapsed_minutes = 0, elapsed_seconds = 0;

/**
 * @struct GNRMC
 * @brief Estructura para almacenar datos NMEA del tipo GNRMC (Recommended Minimum Specific GPS/Transit Data).
 */
typedef struct {
    int Time_H;       ///< Hora del GPS
    int Time_M;       ///< Minuto del GPS
    int Time_S;       ///< Segundo del GPS
    int Status;       ///< Estado de posicionamiento (1: Posicionado, 0: No posicionado)
    double Lat;       ///< Latitud en formato decimal
    char Lat_area;    ///< Hemisferio de latitud ('N' o 'S')
    double Lon;       ///< Longitud en formato decimal
    char Lon_area;    ///< Hemisferio de longitud ('E' o 'W')
} GNRMC;

/** @brief Estructura global para almacenar la información GPS actual */
GNRMC GPS;

/**
 * @brief Configura los pines de la UART 0 para el módulo GPS.
 */
void configurePinUart() {
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

/**
 * @brief Manejador de interrupción UART.
 *
 * Captura el carácter recibido en UART0 y activa la bandera de recepción.
 */
void uartHandler(void) {
    data = uart_getc(UART_ID);   ///< Leer carácter recibido
    uart_flag = true;            ///< Activar bandera de interrupción
}

/**
 * @brief Configura la interrupción UART para la UART0.
 */
void configIntUart() {
    uart_set_irq_enables(UART_ID, true, false);               ///< Habilita interrupción por recepción
    irq_set_exclusive_handler(UART0_IRQ, uartHandler);        ///< Asocia handler exclusivo
    irq_set_enabled(UART0_IRQ, true);                         ///< Habilita IRQ UART0
    uart_set_irq_enables(UART_ID, true, false);               ///< Reafirma habilitación
}

/**
 * @brief Procesa una sentencia NMEA tipo $GNRMC y actualiza la estructura GPS.
 *
 * @param sentence Cadena de texto con la sentencia NMEA completa.
 */
void process_nmea_sentence(const char *sentence) {
    // Verificar si la sentencia es GNRMC
    if (strncmp(sentence, "$GNRMC,", 7) == 0) {
        char *token = strtok((char *)sentence, ",");
        int count = 0;

        while (token != NULL) {
            switch (count) {
                case 1:  ///< Hora GPS en formato hhmmss
                    sscanf(token, "%2d%2d%2d", &GPS.Time_H, &GPS.Time_M, &GPS.Time_S);
                    break;

                case 2:  ///< Estado: 'A' = activo, 'V' = no activo
                    GPS.Status = (token[0] == 'A') ? 1 : 0;
                    break;

                case 3:  ///< Latitud en formato ddmm.mmmm
                    sscanf(token, "%lf", &GPS.Lat);
                    {
                        int lat_deg = (int)(GPS.Lat / 100);
                        double lat_min = GPS.Lat - (lat_deg * 100);
                        GPS.Lat = lat_deg + (lat_min / 60.0);
                    }
                    break;

                case 4:  ///< Hemisferio de latitud ('N' o 'S')
                    GPS.Lat_area = token[0];
                    if (GPS.Lat_area == 'S') {
                        GPS.Lat *= -1;
                    }
                    break;

                case 5:  ///< Longitud en formato dddmm.mmmm
                    sscanf(token, "%lf", &GPS.Lon);
                    {
                        int lon_deg = (int)(GPS.Lon / 100);
                        double lon_min = GPS.Lon - (lon_deg * 100);
                        GPS.Lon = lon_deg + (lon_min / 60.0);
                    }
                    break;

                case 6:  ///< Hemisferio de longitud ('E' o 'W')
                    GPS.Lon_area = token[0];
                    if (GPS.Lon_area == 'W') {
                        GPS.Lon *= -1;
                    }
                    break;
            }

            token = strtok(NULL, ",");
            count++;
        }

        // Guarda la hora inicial cuando el GPS se posiciona por primera vez
        if (GPS.Status == 1 && shora == 0 && sminuto == 0 && ssegundo == 0) {
            shora = GPS.Time_H;
            sminuto = GPS.Time_M;
            ssegundo = GPS.Time_S;
        }

        // Calcula el tiempo transcurrido desde que el GPS obtuvo señal
        elapsed_hours = GPS.Time_H - shora;
        elapsed_minutes = GPS.Time_M - sminuto;
        elapsed_seconds = GPS.Time_S - ssegundo;

        if (elapsed_seconds < 0) {
            elapsed_seconds += 60;
            elapsed_minutes--;
        }

        if (elapsed_minutes < 0) {
            elapsed_minutes += 60;
            elapsed_hours--;
        }

        /*
        printf("Hora: %02d:%02d:%02d, Latitud: %.6lf %c, Longitud: %.6lf %c, Estado: %s\n",
               GPS.Time_H, GPS.Time_M, GPS.Time_S,
               GPS.Lat, GPS.Lat_area,
               GPS.Lon, GPS.Lon_area,
               (GPS.Status == 1) ? "Posicionado" : "No posicionado");
        */
    }
}
