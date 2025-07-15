/**
 * @file eeprom.h
 * @brief Lectura y escritura en EEPROM con generación separada de ruta y puntos con dB.
 */

#include <stdio.h>
#include "formatKML.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <string.h>

#define ADD_EEPROM 0x50
#define SDA_EEPROM 16
#define SCL_EEPROM 17

struct Coordenadas {
    double dato1;  // Latitud
    double dato2;  // Longitud
    float dB;
};

uint8_t read_buffer[16];
uint8_t nwrite_buffer[18];
uint16_t readAddr = 0, startAddr = 0;

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

void writeEeprom(uint16_t current_address, struct Coordenadas coordenadas) {
    uint8_t write_buffer[1 + sizeof(struct Coordenadas)];
    uint8_t device_addr = 0x50 | ((current_address >> 8) & 0x07);
    uint8_t mem_addr = current_address & 0xFF;

    write_buffer[0] = mem_addr;
    memcpy(&write_buffer[1], &coordenadas, sizeof(struct Coordenadas));

    i2c_write_blocking(i2c_default, device_addr, write_buffer, sizeof(write_buffer), false);
}

void readEepromOnlyPoint(uint16_t address) {
    uint8_t address_buffer[1];
    uint8_t read_buffer[sizeof(struct Coordenadas)];

    uint8_t device_addr = 0x50 | ((address >> 8) & 0x07);
    uint8_t mem_addr = address & 0xFF;
    address_buffer[0] = mem_addr;

    i2c_write_blocking(i2c_default, device_addr, address_buffer, 1, true);
    i2c_read_blocking(i2c_default, device_addr, read_buffer, sizeof(read_buffer), false);

    struct Coordenadas coordenadas;
    memcpy(&coordenadas, &read_buffer[0], sizeof(struct Coordenadas));

    if (coordenadas.dato1 != 0 && coordenadas.dato2 != 0) {
        KMLLinePoint(coordenadas.dato2, coordenadas.dato1);
    }
}

void readEepromOnlyPlacemark(uint16_t address) {
    uint8_t address_buffer[1];
    uint8_t read_buffer[sizeof(struct Coordenadas)];

    uint8_t device_addr = 0x50 | ((address >> 8) & 0x07);
    uint8_t mem_addr = address & 0xFF;
    address_buffer[0] = mem_addr;

    i2c_write_blocking(i2c_default, device_addr, address_buffer, 1, true);
    i2c_read_blocking(i2c_default, device_addr, read_buffer, sizeof(read_buffer), false);

    struct Coordenadas coordenadas;
    memcpy(&coordenadas, &read_buffer[0], sizeof(struct Coordenadas));

    if (coordenadas.dato1 != 0 && coordenadas.dato2 != 0 && coordenadas.dB != 0) {
        KMLPlacemark(coordenadas.dato2, coordenadas.dato1, coordenadas.dB);
    }
}

void nDataEeprom(int n, uint16_t posicion) {
    if (posicion >= 2048) {
        printf("Error: La posición excede la capacidad de la EEPROM.\n");
        return;
    }

    int datosAlmacenados = (posicion / sizeof(struct Coordenadas)) + 1;

    if (n > datosAlmacenados) {
        n = datosAlmacenados;
        startAddr = 0;
    } else {
        startAddr = posicion - (n * sizeof(struct Coordenadas));
    }

    // Imprimir solo ruta
    KMLHeader();
    uint16_t addr = startAddr;
    for (int i = 0; i < n; i++) {
        readEepromOnlyPoint(addr);
        addr += sizeof(struct Coordenadas);
    }
    KMLFooter();

    // Imprimir placemarks después de cerrar la ruta
    addr = startAddr;
    for (int i = 0; i < n; i++) {
        readEepromOnlyPlacemark(addr);
        addr += sizeof(struct Coordenadas);
    }
}