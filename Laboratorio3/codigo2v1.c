#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"

#define PWM_GPIO 0
#define DIR_PIN1 1
#define DIR_PIN2 2
#define PWM_FREQ 1000    // 1 kHz PWM
#define DUTY_STEP 1      // Paso 1%

#ifndef clk_sys
#define clk_sys 1
#endif

volatile int duty_cycle = 0;
volatile bool increase = true;

uint slice_num;
uint32_t pwm_wrap_global = 0;

alarm_id_t alarm_id;

// Configura dirección fija para que el motor gire hacia adelante
void setup_direction() {
    gpio_init(DIR_PIN1);
    gpio_set_dir(DIR_PIN1, GPIO_OUT);
    gpio_put(DIR_PIN1, 1);

    gpio_init(DIR_PIN2);
    gpio_set_dir(DIR_PIN2, GPIO_OUT);
    gpio_put(DIR_PIN2, 0);
}

// Configura hardware PWM en gpio con freq y duty inicial
void setup_hardware_pwm(uint gpio, uint freq_hz, int duty_percent) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(gpio);
    uint chan = pwm_gpio_to_channel(gpio);
    uint32_t clk_freq = clock_get_hz(clk_sys);
    pwm_wrap_global = clk_freq / freq_hz - 1;
    pwm_set_wrap(slice_num, pwm_wrap_global);
    pwm_set_chan_level(slice_num, chan, pwm_wrap_global * duty_percent / 100);
    pwm_set_enabled(slice_num, true);
}

// Actualiza duty cycle del hardware PWM
void update_duty_cycle(int duty_percent) {
    uint chan = pwm_gpio_to_channel(PWM_GPIO);
    pwm_set_chan_level(slice_num, chan, pwm_wrap_global * duty_percent / 100);
}

// Modo polling: PWM por software con busy wait
void pwm_polling(uint gpio) {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_OUT);

    while (true) {
        for (int dc = 0; dc <= 100; dc += DUTY_STEP) {
            duty_cycle = dc;
            uint32_t period_us = 1000000 / PWM_FREQ;
            uint32_t high_us = period_us * duty_cycle / 100;
            uint32_t low_us = period_us - high_us;
            for (int i = 0; i < 100; i++) {
                gpio_put(gpio, 1);
                busy_wait_us(high_us);
                gpio_put(gpio, 0);
                busy_wait_us(low_us);
            }
        }
        for (int dc = 100; dc >= 0; dc -= DUTY_STEP) {
            duty_cycle = dc;
            uint32_t period_us = 1000000 / PWM_FREQ;
            uint32_t high_us = period_us * duty_cycle / 100;
            uint32_t low_us = period_us - high_us;
            for (int i = 0; i < 100; i++) {
                gpio_put(gpio, 1);
                busy_wait_us(high_us);
                gpio_put(gpio, 0);
                busy_wait_us(low_us);
            }
        }
    }
}

// Callback alarma timer para modo interrupciones
int64_t alarm_callback(alarm_id_t id, void *user_data) {
    if (increase) {
        duty_cycle++;
        if (duty_cycle >= 100) increase = false;
    } else {
        duty_cycle--;
        if (duty_cycle <= 0) increase = true;
    }
    update_duty_cycle(duty_cycle);
    return 50000;  // 50 ms siguiente interrupción
}

// Modo interrupciones: hardware PWM + timer alarma para cambiar duty cycle
void pwm_interrupciones() {
    setup_hardware_pwm(PWM_GPIO, PWM_FREQ, 0);
    increase = true;
    duty_cycle = 0;
    alarm_id = add_alarm_in_us(50000, alarm_callback, NULL, true);

    while (true) {
        tight_loop_contents();
    }
}

// Modo combinado: hardware PWM y cambio duty en bucle con sleep
void pwm_combinado() {
    setup_hardware_pwm(PWM_GPIO, PWM_FREQ, 0);
    increase = true;
    duty_cycle = 0;

    while (true) {
        sleep_ms(50);
        if (increase) {
            duty_cycle++;
            if (duty_cycle >= 100) increase = false;
        } else {
            duty_cycle--;
            if (duty_cycle <= 0) increase = true;
        }
        update_duty_cycle(duty_cycle);
    }
}

int main() {
    stdio_usb_init();

    setup_direction();

    printf("Selecciona modo PWM: 0=polling, 1=interrupciones, 2=combinado\n");
    while (!stdio_usb_connected()) sleep_ms(100);

    int modo = -1;
    char buf[10];
    int idx = 0;

    while (true) {
        int c = getchar_timeout_us(1000);
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '\r' || c == '\n') {
                buf[idx] = 0;
                break;
            } else if (idx < (int)(sizeof(buf) - 1)) {
                buf[idx++] = (char)c;
                putchar(c);
            }
        }
    }

    if (idx > 0 && (buf[0] == '0' || buf[0] == '1' || buf[0] == '2')) {
        modo = buf[0] - '0';
    } else {
        modo = 0;
    }

    printf("\nModo PWM seleccionado: %d\n", modo);

    switch (modo) {
        case 0:
            pwm_polling(PWM_GPIO);
            break;
        case 1:
            pwm_interrupciones();
            break;
        case 2:
            pwm_combinado();
            break;
        default:
            printf("Modo no válido\n");
            break;
    }

    return 0;
}
