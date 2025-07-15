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
    float dB;
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
    uint8_t write_buffer[1 + sizeof(struct Coordenadas)];  // 1 byte para dirección + datos

    uint8_t device_addr = 0x50 | ((current_address >> 8) & 0x07);  // bits A10-A8
    uint8_t mem_addr = current_address & 0xFF;                     // byte inferior (A7-A0)

    write_buffer[0] = mem_addr;  // dirección interna
    memcpy(&write_buffer[1], &coordenadas, sizeof(struct Coordenadas));

    i2c_write_blocking(i2c_default, device_addr, write_buffer, 1 + sizeof(struct Coordenadas), false);
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

    uint8_t device_addr = 0x50 | ((address >> 8) & 0x07);  // bits A10-A8
    uint8_t mem_addr = address & 0xFF;

    address_buffer[0] = mem_addr;

    i2c_write_blocking(i2c_default, device_addr, address_buffer, 1, true);  // enviar dirección interna
    i2c_read_blocking(i2c_default, device_addr, read_buffer, sizeof(read_buffer), false);

    struct Coordenadas coordenadas;
    memcpy(&coordenadas, &read_buffer[0], sizeof(struct Coordenadas));
    if (coordenadas.lat != 0 && coordenadas.lon != 0 && coordenadas.dB != 0) {
        KMLPlacemark(coordenadas.lat, coordenadas.lon, coordenadas.dB);
        KMLLinePoint(coordenadas.lat, coordenadas.lon);
    }
}


/**
 * @brief Lee una cantidad específica de datos de la EEPROM y genera un archivo KML.
 * 
 * @param n Cantidad de datos a leer.
 * @param posicion Posición inicial de lectura en la EEPROM.
 */
void nDataEeprom(int n, uint16_t posicion) {
    ///< Verificamos que posicion no sea mayor a el tamñao de la memoria.
    if (posicion >= 2048) {
        printf("Error: La posición excede la capacidad de la EEPROM.\n");
        return;
    }

    ///< Calcula la cantidad de datos almacenados.
    int datosAlmacenados = (posicion / sizeof(struct Coordenadas)) + 1;

    ///< Si n es mayor que la cantidad de datos almacenados entonces imprimimos la cantidad de datos que se hayan guardado hasta el momento.
    if (n > datosAlmacenados) {
        n = datosAlmacenados;          ///< Hace n igual a la cantidad de datos.
        startAddr=posicion-posicion;   ///< Asigna como direccion de inicio 0.
    }
    else {
        uint16_t nAddr = posicion - (n * sizeof(struct Coordenadas));  ///< Calcula la posicion desde la cual debe empezar a imprimir
        startAddr = nAddr;
    }

    printf("Posicion: %d, cDatos: %d, n: %d, startAddr: %d\n", posicion, datosAlmacenados, n, startAddr);

    KMLHeader();
    for (int i = 0; i < n; i++) {
        readEeprom(startAddr,i);
        startAddr += sizeof(struct Coordenadas);
    }
    KMLFooter();
}
