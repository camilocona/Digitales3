#ifndef RUIDO_H
#define RUIDO_H

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <math.h>

#define ADC_MICROFONO 0

/**
 * @brief Inicializa el ADC para lectura del micrófono.
 */
static inline void init_adc_microfono() {
    adc_init();
    adc_gpio_init(26); // GPIO 26 = ADC0
    adc_select_input(ADC_MICROFONO);
}

/**
 * @brief Lee una muestra única del micrófono.
 * @return Voltaje en escala 0.0 a 3.3
 */
static inline float leer_adc_microfono() {
    uint16_t raw = adc_read(); // 0 a 4095
    return (3.3f * raw) / 4095.0f;
}

/**
 * @brief Calcula los dB a partir del valor RMS
 * @param valor_lineal Valor promedio (RMS) acumulado
 * @return Decibelios
 */
static inline float calcular_dB_promedio(float valor_lineal) {
    return 20.0f * log10f(valor_lineal + 1e-6f); // evita log(0)
}

#endif