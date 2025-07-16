#ifndef RUIDO_H
#define RUIDO_H

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <math.h>

#define ADC_MICROFONO 0
#define NUM_MUESTRAS 16000  // 1600 muestras/seg × 10 s
#define VREF 3.3f
#define ADC_MAX 4095.0f
#define OFFSET_V 1.65f  // Punto medio del MIC-MAX4466

void init_adc_microfono() {
    adc_init();
    adc_gpio_init(26); // GPIO 26 = ADC0
    adc_select_input(ADC_MICROFONO);
}

/**
 * @brief Convierte una lectura del ADC a voltaje real (en voltios).
 */
static inline float adc_to_voltaje(uint16_t muestra) {
    return ((float)muestra * VREF) / ADC_MAX;
}

/**
 * @brief Mide el nivel de ruido durante 10 segundos.
 * Devuelve dB relativos a 1 V RMS.
 */
float medir_ruido_dB() {
    float suma_cuadrados = 0.0f;

    for (int i = 0; i < NUM_MUESTRAS; i++) {
        uint16_t muestra = adc_read();
        float voltaje = adc_to_voltaje(muestra);
        float ac = voltaje - OFFSET_V;  // quitar componente DC
        suma_cuadrados += ac * ac;
        sleep_us(625);
    }

    float rms = sqrtf(suma_cuadrados / NUM_MUESTRAS);
    if (rms < 0.001f) rms = 0.001f;  // evitar log10(0)

    return 20.0f * log10f(rms / 1.0f);  // dB relativos a 1 V RMS
}

/**
 * @brief Mide el nivel de ruido rápidamente (~20ms), ideal para captura continua cada 0.5 s.
 */
float medir_ruido_dB_rapido() {
    float suma_cuadrados = 0.0f;
    const int muestras = 32;

    for (int i = 0; i < muestras; i++) {
        uint16_t muestra = adc_read();
        float voltaje = adc_to_voltaje(muestra);
        float ac = voltaje - OFFSET_V;
        suma_cuadrados += ac * ac;
        sleep_us(625);
    }

    float rms = sqrtf(suma_cuadrados / muestras);
    if (rms < 0.001f) rms = 0.001f;

    return 20.0f * log10f(rms / 1.0f);  // dB relativos a 1 V RMS
}

#endif
