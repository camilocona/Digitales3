#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GPS_UART uart1      // Usamos UART1 para leer los datos del GPS
#define GPS_BAUD 9600
#define GPS_RX_PIN 5       // Pin RX de UART1 (conectado al TX del GPS)
#define GPS_TX_PIN 4       // Pin TX de UART1 (conectado al RX del GPS)

#define SERIAL_UART uart0  // Usamos UART0 para enviar datos a la terminal (COM)
#define PPS_PIN 3          // Pin GPIO donde conectas el PPS del GPS

// Función para configurar UART1 para la comunicación con el GPS
void setup_uart() {
    uart_init(GPS_UART, GPS_BAUD);  // Inicializa UART1 con la velocidad de 9600 baudios
    gpio_set_function(GPS_RX_PIN, GPIO_FUNC_UART);  // Configura el pin RX de UART1
    gpio_set_function(GPS_TX_PIN, GPIO_FUNC_UART);  // Configura el pin TX de UART1

    uart_init(SERIAL_UART, GPS_BAUD);  // Inicializa UART0 para la comunicación con la computadora
    gpio_set_function(0, GPIO_FUNC_UART);  // Configura el pin 0 (TX de UART0) para la salida serial
    gpio_set_function(1, GPIO_FUNC_UART);  // Configura el pin 1 (RX de UART0) para la entrada serial
}

// Función para convertir grados, minutos y segundos a grados decimales
float convert_to_decimal(char *coord, char direction) {
    int degrees;
    float minutes;
    
    // Separar grados y minutos
    sscanf(coord, "%2d%f", &degrees, &minutes);
    
    // Convertir a grados decimales
    float decimal = degrees + (minutes / 60);
    
    // Si la dirección es sur o oeste, hacer el valor negativo
    if (direction == 'S' || direction == 'W') {
        decimal = -decimal;
    }
    
    return decimal;
}

// Función para leer los datos del GPS y extraer coordenadas
void read_gps_data() {
    static char gps_data[100];
    static int idx = 0;

    while (true) {
        if (uart_is_readable(GPS_UART)) {  // Si hay datos en UART1
            char byte = uart_getc(GPS_UART);  // Lee el byte desde UART1
            
            // Recoger los datos hasta que se complete una sentencia
            if (byte == '\n') {
                gps_data[idx] = '\0';  // Terminar la cadena
                idx = 0;

                // Mostrar los datos recibidos en UART0 (en la terminal serial)
                printf("Datos NMEA: %s\n", gps_data);

                // Si es una sentencia $GNGGA o $GNRMC, extraemos la latitud y longitud
                if (strncmp(gps_data, "$GNGGA", 6) == 0 || strncmp(gps_data, "$GNRMC", 6) == 0) {
                    char lat[10], lon[11], lat_dir, lon_dir;
                    float latitude, longitude;

                    // Buscar la latitud y longitud
                    if (sscanf(gps_data, "$GNGGA,%*f,%9s,%c,%10s,%c", lat, &lat_dir, lon, &lon_dir) == 4 ||
                        sscanf(gps_data, "$GNRMC,%*f,%c,%9s,%c,%10s,%c", &lat_dir, lat, &lat_dir, lon, &lon_dir) == 5) {

                        // Convertir latitud y longitud a grados decimales
                        latitude = convert_to_decimal(lat, lat_dir);
                        longitude = convert_to_decimal(lon, lon_dir);

                        // Mostrar la ubicación en la terminal
                        printf("Ubicación: %.6f, %.6f\n", latitude, longitude);
                    }
                }
            } else {
                gps_data[idx++] = byte;  // Almacenar el byte recibido
            }
        }
    }
}

// Función de interrupción para manejar el pulso PPS
void pps_interrupt_handler(uint gpio, uint32_t events) {
    printf("¡Pulsación PPS detectada!\n");
}

int main() {
    stdio_init_all();

    // Configuración de UART1 para el GPS y UART0 para la salida serial
    setup_uart();

    // Configuración del pin PPS como entrada
    gpio_init(PPS_PIN);
    gpio_set_dir(PPS_PIN, GPIO_IN);
    
    // Configurar la interrupción para detectar flancos ascendentes (cuando el PPS ocurre)
    gpio_set_irq_enabled_with_callback(PPS_PIN, GPIO_IRQ_EDGE_RISE, true, &pps_interrupt_handler);

    printf("Esperando datos del GPS y señales PPS...\n");

    // Hilo principal para leer datos GPS y detectar PPS
    while (true) {
        // Leer los datos del GPS desde UART1 y enviarlos a través de UART0
        read_gps_data();
    }

    return 0;
}
