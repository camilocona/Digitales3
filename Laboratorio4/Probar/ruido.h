#ifndef RUIDO_H
#define RUIDO_H

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <math.h>

#define ADC_MICROFONO 0
#define NUM_MUESTRAS 16000  // 1600 muestras/seg × 10s

void init_adc_microfono() {
    adc_init();
    adc_gpio_init(26); // GPIO 26 = ADC0
    adc_select_input(ADC_MICROFONO);
}

/**
 * @brief Mide el nivel de ruido durante 10 segundos (completo).
 */
float medir_ruido_dB() {
    float suma_cuadrados = 0;
    for (int i = 0; i < NUM_MUESTRAS; i++) {
        uint16_t muestra = adc_read();
        float voltaje = (float)muestra;
        suma_cuadrados += voltaje * voltaje;
        sleep_us(625);
    }

    float rms = sqrt(suma_cuadrados / NUM_MUESTRAS);
    if (rms < 1.0f) rms = 1.0f;
    return 20.0f * log10f(rms / 4095.0f);
}

/**
 * @brief Mide el nivel de ruido rápidamente (~20ms), ideal para captura continua cada 0.5 s.
 */
float medir_ruido_dB_rapido() {
    float suma_cuadrados = 0;
    const int muestras = 32;

    for (int i = 0; i < muestras; i++) {
        uint16_t muestra = adc_read();
        float voltaje = (float)muestra;
        suma_cuadrados += voltaje * voltaje;
        sleep_us(625);
    }

    float rms = sqrtf(suma_cuadrados / muestras);
    if (rms < 1.0f) rms = 1.0f;

    return 20.0f * log10f(rms / 4095.0f);
}

#endif