#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"

#define IN1 1
#define IN2 2
#define ENA 0
#define SENSOR_PIN 10
#define PULSOS_POR_REV 20
#define MAX_MUESTRAS 5000
#define N_PULSOS_MEDICION 10

// Buffers
uint32_t timestamp[MAX_MUESTRAS];
int pwmBuffer[MAX_MUESTRAS];
float rpmBuffer[MAX_MUESTRAS];
int idx = 0;

// PWM
uint32_t pwm_wrap = 0;

// Variables para modo IRQ
volatile float rpm_irq = 0;
volatile uint32_t contador_local = 0;
volatile uint32_t tiempo_inicio = 0;

// Combinado
volatile uint32_t pulse_count = 0;

// Modo de medicion: 0=polling, 1=irq, 2=combinado
int modo_medicion = 0;

void setup_pwm(uint gpio_pwm, uint freq_hz, float duty_percent) {
    gpio_set_function(gpio_pwm, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio_pwm);
    uint chan = pwm_gpio_to_channel(gpio_pwm);
    uint32_t clk = clock_get_hz(clk_sys);
    pwm_wrap = clk / freq_hz - 1;
    pwm_set_wrap(slice, pwm_wrap);
    pwm_set_chan_level(slice, chan, (uint32_t)(pwm_wrap * duty_percent / 100.0f));
    pwm_set_enabled(slice, true);
}

void set_pwm_duty(uint gpio_pwm, float duty_percent) {
    uint slice = pwm_gpio_to_slice_num(gpio_pwm);
    uint chan = pwm_gpio_to_channel(gpio_pwm);
    pwm_set_chan_level(slice, chan, (uint32_t)(pwm_wrap * duty_percent / 100.0f));
}

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == SENSOR_PIN && (events & GPIO_IRQ_EDGE_RISE)) {
        uint32_t ahora = time_us_32();

        if (modo_medicion == 1) {
            if (contador_local == 0) {
                tiempo_inicio = ahora;
            }

            contador_local++;

            if (contador_local >= N_PULSOS_MEDICION) {
                uint32_t delta = ahora - tiempo_inicio;
                if (delta > 0) {
                    rpm_irq = (1e6f / delta) * N_PULSOS_MEDICION / PULSOS_POR_REV * 60.0f;
                }
                contador_local = 0;
            }
        }

        else if (modo_medicion == 2) {
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
    setup_pwm(ENA, 10000, 0.0f);

    gpio_set_irq_enabled_with_callback(SENSOR_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    while (!stdio_usb_connected()) sleep_ms(100);
    printf("Selecciona modo de medicion: 0=polling, 1=irq, 2=combinado\n");

    char buf[10]; int idx_b = 0;
    while (true) {
        int c = getchar_timeout_us(1000);
        if (c != PICO_ERROR_TIMEOUT) {
            if (c == '\r' || c == '\n') {
                buf[idx_b] = 0;
                break;
            } else if (idx_b < (int)(sizeof(buf)-1)) {
                buf[idx_b++] = (char)c;
                putchar(c);
            }
        }
    }

    if (buf[0] == '0' || buf[0] == '1' || buf[0] == '2')
        modo_medicion = buf[0] - '0';

    const int escalones[] = {0, 20, 40, 60, 80, 100, 80, 60, 40, 20, 0};
    const int num_escalones = sizeof(escalones)/sizeof(escalones[0]);

    absolute_time_t inicio = get_absolute_time();
    uint32_t tiempo_muestra = time_us_32();
    uint32_t last_calc_time = to_ms_since_boot(get_absolute_time());

    // Encabezado CSV por serial
    printf("\ntimestamp_ms,pwm_percent,rpm\n");

    for (int i = 0; i < num_escalones; i++) {
        set_pwm_duty(ENA, escalones[i]);
        uint32_t t_inicio = to_ms_since_boot(get_absolute_time());
        pulse_count = 0;
        contador_local = 0;

        while ((to_ms_since_boot(get_absolute_time()) - t_inicio) < 2000) {
            uint32_t ahora = time_us_32();
            if ((ahora - tiempo_muestra) >= 4000 && idx < MAX_MUESTRAS) {
                float rpm = 0;

                if (modo_medicion == 0) {
                    static bool estado_ant = false;
                    static uint32_t tiempo_ant = 0;
                    bool estado = gpio_get(SENSOR_PIN);
                    if (estado && !estado_ant) {
                        uint32_t t = time_us_32();
                        uint32_t delta = t - tiempo_ant;
                        if (delta > 0)
                            rpm = (1e6f / delta / PULSOS_POR_REV) * 60.0f;
                        tiempo_ant = t;
                    }
                    estado_ant = estado;
                }

                else if (modo_medicion == 1) {
                    rpm = rpm_irq;
                }

                else if (modo_medicion == 2) {
                    uint32_t t_ms = to_ms_since_boot(get_absolute_time());
                    uint32_t delta = t_ms - last_calc_time;
                    if (delta > 0) {
                        rpm = ((float)pulse_count / PULSOS_POR_REV) * (60.0f / (delta / 1000.0f));
                        pulse_count = 0;
                        last_calc_time = t_ms;
                    }
                }

                // Guardar solo si tiene sentido fisico o si PWM=0
                if (rpm > 0.0f || escalones[i] == 0) {
                    timestamp[idx] = to_ms_since_boot(get_absolute_time()) - to_ms_since_boot(inicio);
                    pwmBuffer[idx] = escalones[i];
                    rpmBuffer[idx] = rpm;

                    // Imprimir al serial en formato CSV
                    printf("%lu,%d,%.2f\n", timestamp[idx], pwmBuffer[idx], rpmBuffer[idx]);

                    idx++;
                }

                tiempo_muestra = ahora;
            }
        }
    }

    printf("Secuencia completada.\n");
    return 0;
}
