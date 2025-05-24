#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h" 

#define IN1 1
#define IN2 2
#define ENA 0  // PWM pin
#define SENSOR_PIN 10
#define PULSOS_POR_REV 20

// Inicializa PWM en el pin dado con un duty cycle inicial (0-100 %)
void setup_pwm(uint gpio_pwm, uint freq_hz, float duty_percent) {
    gpio_set_function(gpio_pwm, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio_pwm);
    uint chan = pwm_gpio_to_channel(gpio_pwm);

    uint32_t clk_freq = clock_get_hz(clk_sys);
    uint32_t wrap = clk_freq / freq_hz;

    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, chan, (uint32_t)(wrap * duty_percent / 100.0f));
    pwm_set_enabled(slice_num, true);
}

// Cambia duty cycle (0-100 %)
void set_pwm_duty(uint gpio_pwm, float duty_percent) {
    uint slice_num = pwm_gpio_to_slice_num(gpio_pwm);
    uint chan = pwm_gpio_to_channel(gpio_pwm);
    uint32_t wrap = pwm_gpio_to_slice_num(slice_num);
    pwm_set_chan_level(slice_num, chan, (uint32_t)(wrap * duty_percent / 100.0f));
}

int main() {
    stdio_usb_init();
    gpio_init(IN1); gpio_set_dir(IN1, GPIO_OUT);
    gpio_init(IN2); gpio_set_dir(IN2, GPIO_OUT);
    gpio_init(SENSOR_PIN); gpio_set_dir(SENSOR_PIN, GPIO_IN);
    gpio_pull_down(SENSOR_PIN);

    // Dirección del motor: adelante
    gpio_put(IN1, 1);
    gpio_put(IN2, 0);

    // PWM setup (por ejemplo: 10 kHz, 50% de velocidad inicial)
    setup_pwm(ENA, 10000, 100.0f);  // ENA → GPIO 6

    bool estado_anterior = false;
    uint32_t tiempo_anterior = time_us_32();
    float rpm = 0;

    while (true) {
        bool estado_actual = gpio_get(SENSOR_PIN);

        // Detectar flanco de subida
        if (estado_actual && !estado_anterior) {
            uint32_t tiempo_actual = time_us_32();
            uint32_t delta_us = tiempo_actual - tiempo_anterior;

            if (delta_us > 0) {
                float revs_por_seg = 1e6f / delta_us / PULSOS_POR_REV;
                rpm = revs_por_seg * 60.0f;
                printf("RPM: %.2f\n", rpm);
            }

            tiempo_anterior = tiempo_actual;
        }

        estado_anterior = estado_actual;

        // Cambiar la velocidad aquí si deseas probar:
         //set_pwm_duty(ENA, 70.0f);  // aumenta velocidad a 70%

        sleep_ms(1);  // antirrebote / control de CPU
    }

    return 0;
}
