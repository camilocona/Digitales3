/**
 * @file eeprom.h
 * @brief Programa para lectura y escritura en una EEPROM a través de I2C y generación de archivos KML.
 * 
 * Este módulo permite:
 * - Configurar el bus I2C.
 * - Escribir y leer estructuras de datos de tipo `Coordenadas` en una EEPROM externa.
 * - Convertir los datos almacenados en formatos geoespaciales KML para visualización.
 * 
 * @authors
 * - Camilo Andres Anacona Anacona
 * - Maria Valentina Quiroga Alzate
 */

#include <stdio.h>              ///< Entrada/salida estándar
#include "formatKML.h"         ///< Funciones auxiliares para generar archivos KML
#include "pico/stdlib.h"       ///< Funciones estándar para la Raspberry Pi Pico
#include "hardware/i2c.h"      ///< Control del bus I2C
#include <string.h>            ///< Funciones de manipulación de memoria
#include <math.h>              ///< Funciones matemáticas (validación de NaN, fabs)

/** @brief Dirección base de la EEPROM en el bus I2C */
#define ADD_EEPROM 0x50

/** @brief Pin GPIO para línea SDA del I2C */
#define SDA_EEPROM 16

/** @brief Pin GPIO para línea SCL del I2C */
#define SCL_EEPROM 17

/** @brief Tamaño en bytes de la estructura Coordenadas */
#define COORDENADAS_SIZE 20

/**
 * @struct Coordenadas
 * @brief Estructura que almacena una medición georreferenciada con nivel de ruido.
 */
struct Coordenadas {
    double dato1;  ///< Latitud
    double dato2;  ///< Longitud
    float dB;      ///< Nivel de ruido en decibeles (dB)
} __attribute__((packed)); ///< Se empaca sin relleno de memoria

/** @brief Búfer para lectura desde la EEPROM */
uint8_t read_buffer[COORDENADAS_SIZE];

/** @brief Búfer para escritura en EEPROM (incluye byte de dirección + datos) */
uint8_t nwrite_buffer[1 + COORDENADAS_SIZE];

/** @brief Dirección actual de lectura y de inicio de lectura */
uint16_t readAddr = 0, startAddr = 0;

/**
 * @brief Configura el bus I2C para comunicación con la EEPROM externa.
 * 
 * Inicializa los pines SDA y SCL, activa resistencias pull-up y ajusta la velocidad a 100 kHz.
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
 * @brief Escribe una estructura de coordenadas en una posición de memoria EEPROM.
 * 
 * @param current_address Dirección interna de EEPROM donde se escribirá.
 * @param coordenadas Estructura de tipo Coordenadas con latitud, longitud y nivel de ruido.
 */
void writeEeprom(uint16_t current_address, struct Coordenadas coordenadas) {
    uint8_t write_buffer[1 + COORDENADAS_SIZE];

    uint8_t device_addr = 0x50 | ((current_address >> 8) & 0x07);
    uint8_t mem_addr = current_address & 0xFF;

    write_buffer[0] = mem_addr;
    memcpy(&write_buffer[1], &coordenadas, COORDENADAS_SIZE);

    i2c_write_blocking(i2c_default, device_addr, write_buffer, sizeof(write_buffer), false);
    printf("EEPROM Write: %.2f dB @ %.6f, %.6f\n", coordenadas.dB, coordenadas.dato1, coordenadas.dato2);
}

/**
 * @brief Lee una posición de la EEPROM y genera un punto KML de línea (sin dB).
 * 
 * @param address Dirección interna de la EEPROM desde donde leer.
 */
void readEepromOnlyPoint(uint16_t address) {
    uint8_t address_buffer[1];
    uint8_t read_buffer[COORDENADAS_SIZE];

    uint8_t device_addr = 0x50 | ((address >> 8) & 0x07);
    uint8_t mem_addr = address & 0xFF;
    address_buffer[0] = mem_addr;

    i2c_write_blocking(i2c_default, device_addr, address_buffer, 1, true);
    i2c_read_blocking(i2c_default, device_addr, read_buffer, COORDENADAS_SIZE, false);

    struct Coordenadas coordenadas;
    memcpy(&coordenadas, &read_buffer[0], COORDENADAS_SIZE);

    if (!isnan(coordenadas.dato1) && !isnan(coordenadas.dato2) &&
        coordenadas.dato1 >= 6.0 && coordenadas.dato1 <= 7.0 &&
        coordenadas.dato2 <= -75.0 && coordenadas.dato2 >= -76.0) {
        KMLLinePoint(coordenadas.dato2, coordenadas.dato1);  // lon, lat
    }
}

/**
 * @brief Lee una posición de la EEPROM y genera un punto KML tipo placemark con dB.
 * 
 * @param address Dirección interna de la EEPROM desde donde leer.
 */
void readEepromOnlyPlacemark(uint16_t address) {
    uint8_t address_buffer[1];
    uint8_t read_buffer[COORDENADAS_SIZE];

    uint8_t device_addr = 0x50 | ((address >> 8) & 0x07);
    uint8_t mem_addr = address & 0xFF;
    address_buffer[0] = mem_addr;

    i2c_write_blocking(i2c_default, device_addr, address_buffer, 1, true);
    i2c_read_blocking(i2c_default, device_addr, read_buffer, COORDENADAS_SIZE, false);

    struct Coordenadas coordenadas;
    memcpy(&coordenadas, &read_buffer[0], COORDENADAS_SIZE);

    if (!isnan(coordenadas.dato1) && !isnan(coordenadas.dato2) &&
        coordenadas.dato1 >= 6.0 && coordenadas.dato1 <= 7.0 &&
        coordenadas.dato2 <= -75.0 && coordenadas.dato2 >= -76.0 &&
        fabs(coordenadas.dB) > 0.01f) {
        KMLPlacemark(coordenadas.dato2, coordenadas.dato1, coordenadas.dB);
    }
}

/**
 * @brief Lee y genera un archivo KML con los últimos `n` datos almacenados en EEPROM.
 * 
 * @param n Número de registros a leer.
 * @param posicion Dirección final hasta donde leer en EEPROM.
 */
void nDataEeprom(int n, uint16_t posicion) {
    if (posicion >= 2048) {
        printf("Error: La posición excede la capacidad de la EEPROM.\n");
        return;
    }

    int datosAlmacenados = (posicion / COORDENADAS_SIZE);
    if (n > datosAlmacenados) {
        n = datosAlmacenados;
        startAddr = 0;
    } else {
        startAddr = posicion - (n * COORDENADAS_SIZE);
    }

    KMLHeader();

    uint16_t addr = startAddr;
    for (int i = 0; i < n; i++) {
        readEepromOnlyPoint(addr);
        addr += COORDENADAS_SIZE;
    }

    // Cierre explícito de la sección LineString del KML
    printf("    </coordinates>\n");
    printf("   </LineString>\n");
    printf("  </Placemark>\n");

    addr = startAddr;
    for (int i = 0; i < n; i++) {
        readEepromOnlyPlacemark(addr);
        addr += COORDENADAS_SIZE;
    }

    // Cierre completo del documento KML
    printf(" </Document>\n");
    printf("</kml>\n");
}
