/**
 * @file gps.h
 * @brief Programa para la gestión de datos NMEA de un módulo GPS utilizando la UART.
 */
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include <stdio.h>
#include <string.h>

#define UART_ID uart0   ///< UART a utilizar
#define BAUD_RATE 9600  ///< velocidad de transmisión
#define BUFFSIZE 256    ///< tamaño del búfer

#define UART_TX_PIN 0
#define UART_RX_PIN 1

volatile bool uart_flag = false; ///< Bandera que indica si se ha recibido un carácter por UART.
volatile char data;              ///< Variable para almacenar el carácter recibido por UART.

/// @brief Variables para el manejo del tiempo que lleva el gps activo.
volatile int shora=0, sminuto=0, ssegundo=0;
volatile int elapsed_hours=0, elapsed_minutes=0, elapsed_seconds=0; 
/**
 * @struct GNRMC
 * @brief Estructura para almacenar datos NMEA del tipo GNRMC.
 */
typedef struct {
    int Time_H;    ///< Tiempo en horas.            
    int Time_M;    ///< Tiempo en minutos. 
    int Time_S;    ///< Tiempo en segundos. 
    int Status;    ///< Estado de posicionamiento (1: Posicionado, 0: No posicionado).  
    double Lat;    ///< Latitud en formato decimal.
    char Lat_area; //< Hemisferio de latitud (N o S).
    double Lon;    ///< Longitud en formato decimal.
    char Lon_area; ///< Hemisferio de longitud (E o W).
} GNRMC;

GNRMC GPS;        ///< Estructura para almacenar datos NMEA del tipo GNRMC.

/**
 * @brief Configura los pines de la UART 0.
 */
void configurePinUart(){
    /// Configurar la UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
}

/**
 * @brief Manejador de interrupción UART.
 * Almacena el carácter que ingresa por el puerto UART0 y activa la bandera de interrupción.
 */
void uartHandler(void) {
    data = uart_getc(UART_ID);  ///< Almacena el caracter que ingreso por el puerto Uart0;
    uart_flag = true;           ///< Cuando se recibe un caracter activamos la flag de la interrupcion.
}

/**
 * @brief Configura la interrupción UART.
 */
void configIntUart(){
    ///< Configurar interrupción UART
    uart_set_irq_enables(UART_ID, true, false);

    ///< Configurar y habilitar la interrupción
    irq_set_exclusive_handler(UART0_IRQ, uartHandler);
    irq_set_enabled(UART0_IRQ, true);

    uart_set_irq_enables(UART_ID, true, false);
}

/**
 * @brief Procesa una sentencia NMEA y actualiza la estructura GPS.
 * @param sentence Sentencia NMEA recibida.
 */
void process_nmea_sentence(const char *sentence) {
    ///< Verificar si la sentencia comienza con $GNRMC
    if (strncmp(sentence, "$GNRMC,", 7) == 0) {
        ///< Tokenizar la sentencia usando la coma como delimitador
        char *token = strtok((char *)sentence, ",");
        int count = 0;

        while (token != NULL) {
            ///< Procesar cada token de la sentencia GNRMC
            switch (count) {
                case 1:  ///< Hora 
                    sscanf(token, "%2d%2d%2d", &GPS.Time_H, &GPS.Time_M, &GPS.Time_S);
                    break;
                case 2:  ///< Estado de posicionamiento (A: Posicionado, V: No posicionado)
                    GPS.Status = (token[0] == 'A') ? 1 : 0;
                    break;
                case 3:  ///< Latitud
                    sscanf(token, "%lf", &GPS.Lat);
                    int lat_deg = (int)(GPS.Lat / 100);
                    double lat_min = GPS.Lat - (lat_deg * 100);
                    GPS.Lat = lat_deg + (lat_min / 60.0);
                    break;
                case 4:  ///< Hemisferio de latitud (N o S)
                    GPS.Lat_area = token[0];
                    if (GPS.Lat_area == 'S') {
                        GPS.Lat *= -1;
                    }

                    break;
                case 5:  ///< Longitud
                    sscanf(token, "%lf", &GPS.Lon);
                    int lon_deg = (int)(GPS.Lon / 100);
                    double lon_min = GPS.Lon - (lon_deg * 100);
                    GPS.Lon = lon_deg + (lon_min / 60.0);
                    break;
                case 6:  ///< Hemisferio de longitud (E o W)
                    GPS.Lon_area = token[0];
                    if (GPS.Lon_area == 'W') {
                        GPS.Lon *= -1;
                    }

                    break;
            }

            ///< Obtener el siguiente token
            token = strtok(NULL, ",");
            count++;
        }

        ///< Guarda el primer dato que da el gps cuando se engancha.
        if (GPS.Status == 1 && shora == 0 && sminuto == 0 && ssegundo == 0) {
            shora = GPS.Time_H;
            sminuto = GPS.Time_M;
            ssegundo = GPS.Time_S;
        }

        ///< Calcula el tiempo transcurrido desde que el GPS se encendió
        elapsed_hours = GPS.Time_H - shora;
        elapsed_minutes = GPS.Time_M - sminuto;
        elapsed_seconds = GPS.Time_S - ssegundo;

        ///< Manejar casos donde los segundos son negativos
        if (elapsed_seconds < 0) {
            elapsed_seconds += 60;
            elapsed_minutes--;
        }
        ///< Manejar casos donde los minutos son negativos
        if (elapsed_minutes < 0) {
            elapsed_minutes += 60;
            elapsed_hours--;
        }

        // Imprimir la información de posición en la consola
        /*printf("Hora: %02d:%02d:%02d, Latitud: %.6lf %c, Longitud: %.6lf %c, Estado: %s\n",
               GPS.Time_H, GPS.Time_M, GPS.Time_S,
               GPS.Lat, GPS.Lat_area,
               GPS.Lon, GPS.Lon_area,
               (GPS.Status == 1) ? "Posicionado" : "No posicionado");*/
    }
}