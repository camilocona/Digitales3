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
    const int N = 1000;
    float suma = 0;
    for (int i = 0; i < N; i++) {
        uint16_t lectura = adc_read();  // 0 - 4095
        float voltaje = lectura * 3.3f / 4095.0f;  // voltaje en voltios
        suma += voltaje * voltaje;
    }

    float vrms = sqrtf(suma / N);

    // Normaliza frente a 1V RMS como referencia arbitraria (ajustable)
    float referencia = 0.003f;  // Puedes calibrar este valor
    float db = 20.0f * log10f(vrms / referencia);

    return db;
}

#endif