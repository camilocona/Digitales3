/**
 * @file timer.h
 * @brief Temporizador de bajo nivel y alarmas periódicas para Raspberry Pi Pico utilizando el SDK de Pico.
 *
 * Este módulo configura dos alarmas independientes utilizando los temporizadores hardware del RP2040.
 * Permite activar funciones periódicamente por interrupción, útil para sincronización de tareas como muestreo, escritura, etc.
 *
 * @authors
 * - Camilo Andres Anacona Anacona
 * - Maria Valentina Quiroga Alzate
 */

#include <stdio.h>
#include "pico/stdlib.h"         ///< Funciones básicas Pico SDK
#include "hardware/timer.h"      ///< Acceso directo a los temporizadores hardware
#include "hardware/irq.h"        ///< Control de interrupciones

/** @brief Número de la Alarma 1 (usada como temporizador principal) */
#define ALARM_NUM_1 0

/** @brief IRQ asociada a la Alarma 1 */
#define ALARM_IRQ_1 TIMER_IRQ_0

/** @brief Número de la Alarma 2 (usada como temporizador secundario) */
#define ALARM_NUM_2 1

/** @brief IRQ asociada a la Alarma 2 */
#define ALARM_IRQ_2 TIMER_IRQ_1

/** @brief Bandera que indica si la Alarma 1 se ha activado */
static volatile bool alarm_fired_1;

/** @brief Bandera que indica si la Alarma 2 se ha activado */
static volatile bool alarm_fired_2;

/**
 * @brief Programa una alarma para que se active después de un retraso en microsegundos.
 *
 * @param delay_us Tiempo de retraso en microsegundos.
 * @param alarm_num Número de alarma a configurar (0 a 3 según el hardware).
 *
 * @note La función no activa la interrupción, solo configura el momento de disparo.
 */
void alarm_in_us(uint32_t delay_us, uint alarm_num) {
    uint64_t target = timer_hw->timerawl + delay_us;
    timer_hw->alarm[alarm_num] = (uint32_t) target;
}

/**
 * @brief Manejador de interrupción para la Alarma 1.
 *
 * Limpia la bandera de interrupción y activa `alarm_fired_1`.
 * Reprograma la alarma para que vuelva a dispararse cada 4 segundos.
 */
static void alarm_irq_1(void) {
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM_1);
    alarm_fired_1 = true;
    alarm_in_us(4000000, ALARM_NUM_1);  ///< Reprograma para dentro de 4 s
}

/**
 * @brief Manejador de interrupción para la Alarma 2.
 *
 * Limpia la bandera de interrupción y activa `alarm_fired_2`.
 * Reprograma la alarma para que vuelva a dispararse cada 1 segundo.
 */
static void alarm_irq_2(void) {
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM_2);
    alarm_fired_2 = true;
    alarm_in_us(1000000, ALARM_NUM_2);  ///< Reprograma para dentro de 1 s
}

/**
 * @brief Inicializa y configura las alarmas del temporizador.
 *
 * - Establece manejadores de interrupción para dos alarmas.
 * - Habilita sus interrupciones correspondientes.
 * - Programa sus primeros disparos (4 s y 1 s respectivamente).
 *
 * @note Esta función debe ser llamada una vez al inicio del programa.
 */
void init_timer(void) {
    // Configurar Alarma 1
    hw_set_bits(&timer_hw->inte, 1u << ALARM_NUM_1);
    irq_set_exclusive_handler(ALARM_IRQ_1, alarm_irq_1);
    irq_set_enabled(ALARM_IRQ_1, true);

    // Configurar Alarma 2
    hw_set_bits(&timer_hw->inte, 1u << ALARM_NUM_2);
    irq_set_exclusive_handler(ALARM_IRQ_2, alarm_irq_2);
    irq_set_enabled(ALARM_IRQ_2, true);

    // Programar alarmas iniciales
    alarm_in_us(4000000, ALARM_NUM_1);  ///< Primer disparo: 4 s
    alarm_in_us(1000000, ALARM_NUM_2);  ///< Primer disparo: 1 s
}
