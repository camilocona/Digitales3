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
    uint8_t write_buffer[18];

    write_buffer[0] = (current_address >> 8) & 0xFF; ///<Almacena el byte más significativo de la dirección actual en el índice 0 del búfer de escritura.
    write_buffer[1] = current_address & 0xFF;        ///<Almacena el byte menos significativo de la dirección actual en el índice 1 del búfer de escritura.

    /**
     * @brief Utiliza la función memcpy para copiar los datos de la estructura Coordenadas a partir del índice 2 del búfer de escritura. Esto permite que la dirección
     *  y los datos de coordenadas se combinen en un solo bloque de datos en el búfer.
    */ 
    memcpy(&write_buffer[2], &coordenadas, sizeof(struct Coordenadas));

    i2c_write_blocking(i2c_default, ADD_EEPROM, write_buffer, 18, false);
}

/**
 * @brief Lee datos de la EEPROM y genera una entrada KML.
 * 
 * @param address Dirección de lectura en la EEPROM.
 * @param i Índice para el número de datos leídos.
 */
void readEeprom(uint16_t address, int i) {
    uint8_t address_buffer[2];                    ///<Se declara un búfer address_buffer de 2 bytes para almacenar la dirección de lectura que se enviará a la EEPROM.
    address_buffer[0] = (address >> 8) & 0xFF;    ///<Almacena el byte más significativo de la dirección de lectura en el índice 0 del búfer.
    address_buffer[1] = address & 0xFF;           ///<Almacena el byte menos significativo de la dirección de lectura en el índice 1 del búfer.

    i2c_write_blocking(i2c_default, ADD_EEPROM, address_buffer, 2, true); ///<Utiliza la función i2c_write_blocking para enviar la dirección de lectura a la EEPROM.
    i2c_read_blocking(i2c_default, ADD_EEPROM, read_buffer, 16, false);   ///<Utiliza la función i2c_read_blocking para leer datos de la EEPROM. 

    struct Coordenadas coordenadas;
    memcpy(&coordenadas, &read_buffer[0], sizeof(struct Coordenadas));

    /*printf("Item: %d, Dirección: 0x%02X%02X\n", i, address >> 8, address & 0xFF);
    printf("Dato1: %f\n", coordenadas.dato1);
    printf("Dato2: %f\n", coordenadas.dato2);*/
    KMLPlacemark(coordenadas.dato1,coordenadas.dato2);
}

/**
 * @brief Lee una cantidad específica de datos de la EEPROM y genera un archivo KML.
 * 
 * @param n Cantidad de datos a leer.
 * @param posicion Posición inicial de lectura en la EEPROM.
 */
void nDataEeprom(int n, uint16_t posicion) {
    ///< Verificamos que posicion no sea mayor a el tamñao de la memoria.
    if (posicion >= 16 * 1024) {
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

