#ifndef RUIDO_H
#define RUIDO_H

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <math.h>

#define ADC_MICROFONO 0
#define NUM_MUESTRAS 16000  // Ej: 1600 muestras por segundo × 10 segundos

/**
 * @brief Inicializa el ADC para lectura del micrófono.
 */
void init_adc_microfono() {
    adc_init();
    adc_gpio_init(26); // GPIO 26 = ADC0
    adc_select_input(ADC_MICROFONO);
}

/**
 * @brief Mide el nivel de ruido durante 10s y retorna valor en dB (aproximado).
 * @return Nivel de sonido en decibelios.
 */
float medir_ruido_dB() {
    float suma_cuadrados = 0;
    for (int i = 0; i < NUM_MUESTRAS; i++) {
        uint16_t muestra = adc_read();  // Valor de 0 a 4095
        float voltaje = (float)muestra; // Normalizado si se quiere
        suma_cuadrados += voltaje * voltaje;

        sleep_us(625); // 1600 muestras por segundo ≈ 625us por muestra
    }

    float rms = sqrt(suma_cuadrados / NUM_MUESTRAS);
    float db = 20.0f * log10f(rms / 4095.0f);  // Normalizado al valor máx del ADC

    return db;
}

#endif