#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"

#define IN1 1
#define IN2 2
#define ENA 0       // PWM pin
#define SENSOR_PIN 10
#define PULSOS_POR_REV 20

uint32_t pwm_wrap_global = 0;

volatile uint32_t last_time_irq = 0;
volatile float rpm_irq = 0;

volatile uint32_t pulse_count = 0;
uint32_t last_calc_time = 0;
float rpm_combo = 0;

int modo_medicion = 0;

void setup_pwm(uint gpio_pwm, uint freq_hz, float duty_percent) {
    gpio_set_function(gpio_pwm, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio_pwm);
    uint chan = pwm_gpio_to_channel(gpio_pwm);
    uint32_t clk_freq = clock_get_hz(clk_sys);
    uint32_t wrap = clk_freq / freq_hz - 1;
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, chan, (uint32_t)(wrap * duty_percent / 100.0f));
    pwm_set_enabled(slice_num, true);
    pwm_wrap_global = wrap;
}

void set_pwm_duty(uint gpio_pwm, float duty_percent) {
    uint slice_num = pwm_gpio_to_slice_num(gpio_pwm);
    uint chan = pwm_gpio_to_channel(gpio_pwm);
    uint32_t wrap = pwm_wrap_global;
    pwm_set_chan_level(slice_num, chan, (uint32_t)(wrap * duty_percent / 100.0f));
}

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == SENSOR_PIN && (events & GPIO_IRQ_EDGE_RISE)) {
        uint32_t now = time_us_32();
        if (modo_medicion == 1) {
            uint32_t delta = now - last_time_irq;
            if (delta > 0) {
                rpm_irq = (1e6f / delta / PULSOS_POR_REV) * 60.0f;
            }
            last_time_irq = now;
        }
        if (modo_medicion == 2) {
            pulse_count++;
        }
    }
}

int main() {
    stdio_usb_init();
    gpio_init(IN1); gpio_set_dir(IN1, GPIO_OUT);
    gpio_init(IN2); gpio_set_dir(IN2, GPIO_OUT);
    gpio_init(SENSOR_PIN); gpio_set_dir(SENSOR_PIN, GPIO_IN);
    gpio_pull_up(SENSOR_PIN);

    gpio_put(IN1, 1);
    gpio_put(IN2, 0);

    setup_pwm(ENA, 10000, 50.0f);

    bool estado_anterior = false;
    uint32_t tiempo_anterior = time_us_32();
    float rpm_polling = 0;

    gpio_set_irq_enabled_with_callback(SENSOR_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    last_calc_time = to_ms_since_boot(get_absolute_time());

    // Lectura modo mejorada
    char buf[10];
    int idx = 0;

    printf("Selecciona modo de medicion: 0=polling, 1=interrupcion, 2=combinado\n");
    while (!stdio_usb_connected()) sleep_ms(100);

    while (true) {
        int c = getchar_timeout_us(1000);
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '\r' || c == '\n') {
                buf[idx] = 0;
                break;
            } else if (idx < (int)(sizeof(buf)-1)) {
                buf[idx++] = (char)c;
                putchar(c);  // eco para que se vea en terminal
            }
        }
    }

    if (idx > 0 && (buf[0] == '0' || buf[0] == '1' || buf[0] == '2')) {
        modo_medicion = buf[0] - '0';
    } else {
        modo_medicion = 0;
    }

    printf("\nModo seleccionado: %d\n", modo_medicion);

    // Loop principal con impresión controlada
    uint32_t last_print = to_ms_since_boot(get_absolute_time());

    while (true) {
        uint32_t ahora = to_ms_since_boot(get_absolute_time());

        if (modo_medicion == 0) {  // polling
            bool estado_actual = gpio_get(SENSOR_PIN);

            if (estado_actual && !estado_anterior) {
                uint32_t tiempo_actual = time_us_32();
                uint32_t delta_us = tiempo_actual - tiempo_anterior;
                if (delta_us > 0) {
                    float revs_por_seg = 1e6f / delta_us / PULSOS_POR_REV;
                    rpm_polling = revs_por_seg * 60.0f;
                }
                tiempo_anterior = tiempo_actual;
            }
            estado_anterior = estado_actual;

            if (ahora - last_print >= 200) { // imprimir cada 200 ms
                printf("RPM polling: %.2f\n", rpm_polling);
                last_print = ahora;
            }

        } else if (modo_medicion == 1) {  // interrupciones
            if (ahora - last_print >= 200) {
                printf("RPM interrupcion: %.2f\n", rpm_irq);
                last_print = ahora;
            }

        } else if (modo_medicion == 2) {  // combinado
            uint32_t delta_ms = ahora - last_calc_time;
            if (delta_ms >= 500) {
                rpm_combo = ((float)pulse_count / PULSOS_POR_REV) * (60.0f / (delta_ms / 1000.0f));
                pulse_count = 0;
                last_calc_time = ahora;
            }
            if (ahora - last_print >= 200) {
                printf("RPM combinado: %.2f\n", rpm_combo);
                last_print = ahora;
            }
        }
    }
    return 0;
}
