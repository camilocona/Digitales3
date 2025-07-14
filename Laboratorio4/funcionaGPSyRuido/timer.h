/**
 * @file timer.h
 * @brief Temporizador de bajo nivel y Alarma para Raspberry Pi Pico utilizando Pico SDK.
 */

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/irq.h"

#define ALARM_NUM_1 0
#define ALARM_IRQ_1 TIMER_IRQ_0

#define ALARM_NUM_2 1
#define ALARM_IRQ_2 TIMER_IRQ_1

static volatile bool alarm_fired_1;  ///< Bandera para indicar si la Alarma 1 ha sido activada.
static volatile bool alarm_fired_2;  ///< Bandera para indicar si la Alarma 2 ha sido activada.

/**
 * @brief Configura una alarma para activarse después de un retraso específico en microsegundos.
 * 
 * @param delay_us El retraso en microsegundos.
 * @param alarm_num El número de la alarma.
 */
void alarm_in_us(uint32_t delay_us, uint alarm_num) {
    uint64_t target = timer_hw->timerawl + delay_us;
    timer_hw->alarm[alarm_num] = (uint32_t) target;
}

/**
 * @brief Manejador de interrupción de Alarma para la Alarma 1.
 * 
 * Borra la bandera de interrupción, establece la bandera de activación de la alarma y programa la próxima alarma.
 */
static void alarm_irq_1(void) {
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM_1);
    alarm_fired_1 = true;
    alarm_in_us(4000000, ALARM_NUM_1);  
}

/**
 * @brief Manejador de interrupción de Alarma para la Alarma 2.
 * 
 * Borra la bandera de interrupción, establece la bandera de activación de la alarma y programa la próxima alarma.
 */
static void alarm_irq_2(void) {
    hw_clear_bits(&timer_hw->intr, 1u << ALARM_NUM_2);
    alarm_fired_2 = true;
    alarm_in_us(1000000, ALARM_NUM_2);  
}

/**
 * @brief Inicializa el temporizador y las alarmas.
 * 
 * Habilita las interrupciones para ambas alarmas, configura los manejadores de ISR correspondientes
 * y programa los tiempos iniciales de las alarmas.
 * @note Esta función debe llamarse al principio del programa para iniciar el sistema de temporizadores.
 */
void init_timer(void) {
    hw_set_bits(&timer_hw->inte, 1u << ALARM_NUM_1);
    irq_set_exclusive_handler(ALARM_IRQ_1, alarm_irq_1);
    irq_set_enabled(ALARM_IRQ_1, true);

    hw_set_bits(&timer_hw->inte, 1u << ALARM_NUM_2);
    irq_set_exclusive_handler(ALARM_IRQ_2, alarm_irq_2);
    irq_set_enabled(ALARM_IRQ_2, true);

    alarm_in_us(4000000, ALARM_NUM_1);  /// (Alarma 1) para que se dispare después de 4,000,000 microsegundos (4 segundos).
    alarm_in_us(1000000, ALARM_NUM_2);  /// (Alarma 2) para que se dispare después de 1,000,000 microsegundos (1 segundo).
}