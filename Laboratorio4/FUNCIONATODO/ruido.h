/**
 * @file ruido.h
 * @brief Medición del nivel de presión sonora (ruido) utilizando el ADC del RP2040.
 *
 * Este módulo configura un canal ADC conectado a un micrófono electret (como el IC-MAX4466)
 * para calcular el nivel de ruido ambiental en decibelios relativos a 1 V RMS.
 * 
 * Proporciona dos funciones de medición:
 * - Una para capturas completas (10 segundos).
 * - Otra para capturas rápidas (~20 ms), útiles en tiempo real.
 *
 * @authors
 * - Camilo Andres Anacona Anacona
 * - Maria Valentina Quiroga Alzate
 */

#ifndef RUIDO_H
#define RUIDO_H

#include "pico/stdlib.h"       ///< Funciones básicas para Raspberry Pi Pico
#include "hardware/adc.h"      ///< Control del convertidor ADC del RP2040
#include <math.h>              ///< Funciones matemáticas (sqrtf, log10f)

/** @brief Canal ADC utilizado por el micrófono (GPIO 26 = ADC0) */
#define ADC_MICROFONO 0

/** @brief Número de muestras para una medición de 10 segundos */
#define NUM_MUESTRAS 16000  // 1600 muestras/seg × 10 s

/** @brief Voltaje de referencia del ADC */
#define VREF 3.3f

/** @brief Valor máximo del ADC de 12 bits (RP2040) */
#define ADC_MAX 4095.0f

/** @brief Offset del micrófono en voltios (centro de la señal AC) */
#define OFFSET_V 1.65f  // Punto medio del MIC-MAX4466

/**
 * @brief Inicializa el ADC para lectura del micrófono.
 * 
 * Configura el GPIO 26 (canal ADC0) como entrada analógica.
 */
void init_adc_microfono() {
    adc_init();                          ///< Inicializa el sistema ADC
    adc_gpio_init(26);                  ///< Configura GPIO 26 como entrada ADC
    adc_select_input(ADC_MICROFONO);    ///< Selecciona canal ADC0
}

/**
 * @brief Convierte una lectura del ADC a voltaje real (en voltios).
 * 
 * @param muestra Valor crudo del ADC (0–4095).
 * @return Voltaje correspondiente en voltios.
 */
static inline float adc_to_voltaje(uint16_t muestra) {
    return ((float)muestra * VREF) / ADC_MAX;
}

/**
 * @brief Mide el nivel de ruido durante 10 segundos (alta precisión).
 * 
 * Captura 16000 muestras, calcula el valor cuadrático medio (RMS) y lo convierte a decibelios.
 * Los valores son relativos a 1 V RMS como referencia.
 * 
 * @return Nivel de ruido en decibelios (dB).
 */
float medir_ruido_dB() {
    float suma_cuadrados = 0.0f;

    for (int i = 0; i < NUM_MUESTRAS; i++) {
        uint16_t muestra = adc_read();
        float voltaje = adc_to_voltaje(muestra);
        float ac = voltaje - OFFSET_V;  ///< Elimina componente DC
        suma_cuadrados += ac * ac;
        sleep_us(625);                  ///< Muestra cada 625 µs ≈ 1600 Hz
    }

    float rms = sqrtf(suma_cuadrados / NUM_MUESTRAS);
    if (rms < 0.001f) rms = 0.001f;  ///< Evita log10(0)

    return 20.0f * log10f(rms / 1.0f);  ///< Conversión a dB relativos a 1 V RMS
}

/**
 * @brief Mide el nivel de ruido rápidamente (~20 ms).
 * 
 * Captura 32 muestras para una estimación rápida del nivel de ruido,
 * útil para muestreo continuo o respuesta rápida.
 * 
 * @return Nivel de ruido en decibelios (dB).
 */
float medir_ruido_dB_rapido() {
    float suma_cuadrados = 0.0f;
    const int muestras = 32;

    for (int i = 0; i < muestras; i++) {
        uint16_t muestra = adc_read();
        float voltaje = adc_to_voltaje(muestra);
        float ac = voltaje - OFFSET_V;
        suma_cuadrados += ac * ac;
        sleep_us(625);  ///< Intervalo de muestreo
    }

    float rms = sqrtf(suma_cuadrados / muestras);
    if (rms < 0.001f) rms = 0.001f;

    return 20.0f * log10f(rms / 1.0f);  ///< Conversión a dB
}

#endif  // RUIDO_H
