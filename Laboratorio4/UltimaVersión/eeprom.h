/**
 * @file eeprom.h
 * @brief Programa para lectura y escritura en una EEPROM a través de I2C y generación de archivos KML.
 */

#include <stdio.h>
#include "formatKML.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>

#define ADD_EEPROM 0x50  ///< especifica la dirección de la EEPROM en el bus I2C
#define SDA_EEPROM 16
#define SCL_EEPROM 17

/**
 * @struct Coordenadas
 * @brief Estructura para almacenar datos de coordenadas.
 */
struct Coordenadas {
    double dato1;                      ///< Primer dato de coordenada.
    double dato2;                      ///< Segundo dato de coordenada.
    float ruido_dB;
};

uint8_t read_buffer[16];               ///< Búfer para almacenar datos leídos de la EEPROM.
uint8_t nwrite_buffer[18];             ///< Búfer para almacenar datos a escribir en la EEPROM.

uint16_t readAddr = 0, startAddr = 0; ///< Direcciones de lectura y inicio para la EEPROM.

/**
 * @brief Configura el bus I2C y los pines asociados.
 */
void configure_i2c() {
    gpio_init(SDA_EEPROM);
    gpio_init(SCL_EEPROM);
    gpio_set_function(SDA_EEPROM, GPIO_FUNC_I2C);
    gpio_set_function(SCL_EEPROM, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_EEPROM);
    gpio_pull_up(SCL_EEPROM);

    i2c_init(i2c_default, 100000);
    i2c_set_slave_mode(i2c_default, false, ADD_EEPROM);
}

/**
 * @brief Escribe datos en la EEPROM.
 * 
 * @param current_address Dirección actual en la EEPROM.
 * @param coordenadas Estructura que contiene datos de coordenadas.
 */
void writeEeprom(uint16_t current_address, struct Coordenadas coordenadas) {
    uint8_t write_buffer[1 + sizeof(struct Coordenadas)];

    // Dirección de esclavo según bits A10-A8 del address
    uint8_t device_addr = 0x50 | ((current_address >> 8) & 0x07);
    uint8_t mem_addr = current_address & 0xFF;

    write_buffer[0] = mem_addr;  // Dirección interna dentro del bloque
    memcpy(&write_buffer[1], &coordenadas, sizeof(struct Coordenadas));

    i2c_write_blocking(i2c_default, device_addr, write_buffer, sizeof(write_buffer), false);
    sleep_ms(5); // Tiempo para escritura interna en EEPROM
}

/**
 * @brief Lee datos de la EEPROM y genera una entrada KML.
 * 
 * @param address Dirección de lectura en la EEPROM.
 * @param i Índice para el número de datos leídos.
 */
void readEeprom(uint16_t address, int i) {
    uint8_t address_buffer[1];
    uint8_t read_buffer[sizeof(struct Coordenadas)];

    uint8_t device_addr = 0x50 | ((address >> 8) & 0x07);
    uint8_t mem_addr = address & 0xFF;

    address_buffer[0] = mem_addr;

    i2c_write_blocking(i2c_default, device_addr, address_buffer, 1, true);
    i2c_read_blocking(i2c_default, device_addr, read_buffer, sizeof(read_buffer), false);

    struct Coordenadas coordenadas;
    memcpy(&coordenadas, read_buffer, sizeof(struct Coordenadas));

    KMLPlacemark(coordenadas.dato1, coordenadas.dato2, coordenadas.ruido_dB);

    // Para visualización puedes mostrar esto por consola o armar un KML avanzado:
    printf("Dato %d: Dato1 = %.6lf, Dato2 = %.6lf, Ruido = %.2f dB\n", i+1, coordenadas.dato1, coordenadas.dato2, coordenadas.ruido_dB);
}

/**
 * @brief Lee una cantidad específica de datos de la EEPROM y genera un archivo KML.
 * 
 * @param n Cantidad de datos a leer.
 * @param posicion Posición inicial de lectura en la EEPROM.
 */
void nDataEeprom(int n, uint16_t posicion) {
    if (posicion >= 2048) {
        printf("Error: La posición excede la capacidad de la EEPROM.\n");
        return;
    }

    int total_datos = (posicion / sizeof(struct Coordenadas));
    if (n > total_datos) {
        n = total_datos;
        startAddr = 0;
    } else {
        startAddr = posicion - (n * sizeof(struct Coordenadas));
    }

    printf("Posición EEPROM: %d, Datos disponibles: %d, Mostrando últimos: %d\n", posicion, total_datos, n);

    KMLHeader();
    for (int i = 0; i < n; i++) {
        readEeprom(startAddr, i);
        startAddr += sizeof(struct Coordenadas);
    }
    KMLFooter();
}