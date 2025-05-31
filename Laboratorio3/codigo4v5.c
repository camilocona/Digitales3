#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

// Buffers para captura
uint32_t timestamp[MAX_MUESTRAS];
int pwmBuffer[MAX_MUESTRAS];
float rpmBuffer[MAX_MUESTRAS];
int idx = 0;

// PWM
uint32_t pwm_wrap = 0;
int pwm_actual = 0;

// Modo de medición
int modo_medicion = 0; // 0=polling, 1=irq, 2=combinado
bool capturando = false;
bool sistema_activo = true;

// Variables para IRQ
volatile float rpm_irq = 0;
volatile uint32_t contador_local = 0;
volatile uint32_t tiempo_inicio = 0;

// Variables para combinado
volatile uint32_t pulse_count = 0;
uint32_t last_calc_time = 0;

// Variables para polling
bool estado_ant = false;
uint32_t tiempo_ant = 0;

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

void set_pwm_duty(int porcentaje) {
    pwm_actual = porcentaje;
    uint slice = pwm_gpio_to_slice_num(ENA);
    uint chan = pwm_gpio_to_channel(ENA);
    pwm_set_chan_level(slice, chan, (uint32_t)(pwm_wrap * porcentaje / 100.0f));
}

float medir_rpm() {
    if (modo_medicion == 0) {
        bool estado = gpio_get(SENSOR_PIN);
        if (estado && !estado_ant) {
            uint32_t t = time_us_32();
            uint32_t delta = t - tiempo_ant;
            tiempo_ant = t;
            estado_ant = estado;
            if (delta > 0)
                return (1e6f / delta / PULSOS_POR_REV) * 60.0f;
        }
        estado_ant = estado;
        return 0.0f;
    } else if (modo_medicion == 1) {
        return rpm_irq;
    } else {
        uint32_t ahora = to_ms_since_boot(get_absolute_time());
        uint32_t delta = ahora - last_calc_time;
        if (delta >= 500) {
            float rpm = ((float)pulse_count / PULSOS_POR_REV) * (60.0f / (delta / 1000.0f));
            pulse_count = 0;
            last_calc_time = ahora;
            return rpm;
        }
        return 0.0f;
    }
}

void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == SENSOR_PIN && (events & GPIO_IRQ_EDGE_RISE)) {
        uint32_t ahora = time_us_32();
        if (modo_medicion == 1) {
            if (contador_local == 0) tiempo_inicio = ahora;
            contador_local++;
            if (contador_local >= 10) {
                uint32_t delta = ahora - tiempo_inicio;
                if (delta > 0)
                    rpm_irq = (1e6f / delta) * 10 / PULSOS_POR_REV * 60.0f;
                contador_local = 0;
            }
        } else if (modo_medicion == 2) {
            pulse_count++;
        }
    }
}

void captura_por_15s(int pwm_deseado) {
    idx = 0;
    pulse_count = 0;
    last_calc_time = to_ms_since_boot(get_absolute_time());

    set_pwm_duty(pwm_deseado);
    absolute_time_t inicio = get_absolute_time();
    uint32_t tiempo_muestra = time_us_32();

    while (to_ms_since_boot(get_absolute_time()) - to_ms_since_boot(inicio) < 15000 && idx < MAX_MUESTRAS) {
        uint32_t ahora = time_us_32();

        if ((ahora - tiempo_muestra) >= 4000) { // cada 4 ms
            float rpm = medir_rpm();
            uint32_t t_ms = to_ms_since_boot(get_absolute_time());
            timestamp[idx] = t_ms - to_ms_since_boot(inicio);
            pwmBuffer[idx] = pwm_deseado;
            rpmBuffer[idx] = rpm;
            idx++;
            tiempo_muestra = ahora;
        }
    }

    set_pwm_duty(0); // apaga motor

    printf("timestamp_ms,pwm_percent,rpm\n");
    for (int i = 0; i < idx; i++) {
        printf("%lu,%d,%.2f\n", timestamp[i], pwmBuffer[i], rpmBuffer[i]);
    }
    printf("Captura finalizada.\n");
}

void captura_reaccion(int paso_pwm) {
    const int tiempo_entre_pasos = 2000;
    idx = 0;
    capturando = true;

    int pwm = 0;
    absolute_time_t inicio = get_absolute_time();
    printf("timestamp_ms,pwm_percent,rpm\n");

    for (int i = 0; pwm <= 100; i++, pwm = i * paso_pwm) {
        if (pwm > 100) break;
        set_pwm_duty(pwm);
        uint32_t t_inicio = to_ms_since_boot(get_absolute_time());

        while (to_ms_since_boot(get_absolute_time()) - t_inicio < tiempo_entre_pasos) {
            uint32_t ahora = time_us_32();
            static uint32_t t_muestra = 0;
            if (ahora - t_muestra >= 4000 && idx < MAX_MUESTRAS) {
                float rpm = medir_rpm();
                timestamp[idx] = to_ms_since_boot(get_absolute_time()) - to_ms_since_boot(inicio);
                pwmBuffer[idx] = pwm;
                rpmBuffer[idx] = rpm;
                printf("%lu,%d,%.2f\n", timestamp[idx], pwmBuffer[idx], rpmBuffer[idx]);
                idx++;
                t_muestra = ahora;
            }
        }
    }

    for (int i = 100 - paso_pwm; i >= 0; i -= paso_pwm) {
        set_pwm_duty(i);
        uint32_t t_inicio = to_ms_since_boot(get_absolute_time());

        while (to_ms_since_boot(get_absolute_time()) - t_inicio < tiempo_entre_pasos) {
            uint32_t ahora = time_us_32();
            static uint32_t t_muestra = 0;
            if (ahora - t_muestra >= 4000 && idx < MAX_MUESTRAS) {
                float rpm = medir_rpm();
                timestamp[idx] = to_ms_since_boot(get_absolute_time()) - to_ms_since_boot(inicio);
                pwmBuffer[idx] = i;
                rpmBuffer[idx] = rpm;
                printf("%lu,%d,%.2f\n", timestamp[idx], pwmBuffer[idx], rpmBuffer[idx]);
                idx++;
                t_muestra = ahora;
            }
        }
    }

    set_pwm_duty(0); // apagar motor al final
    printf("Secuencia completada.\n");
    capturando = false;
}

void modo_interactivo() {
    char comando[32];
    while (sistema_activo) {
        if (fgets(comando, sizeof(comando), stdin)) {
            if (strncmp(comando, "START", 5) == 0) {
                int paso = atoi(&comando[6]);
                if (paso > 0 && paso <= 100)
                    captura_reaccion(paso);
                else
                    printf("Valor de PWM inválido.\n");
            }
            else if (strncmp(comando, "PWM", 3) == 0) {
                int val = atoi(&comando[4]);
                if (val >= 0 && val <= 100) {
                    printf("PWM recibido: %d %%\n", val);
                    captura_por_15s(val);
                } else {
                    printf("Valor fuera de rango.\n");
                }
            }
            else if (strncmp(comando, "STOP", 4) == 0) {
                set_pwm_duty(0);
                sistema_activo = false;
                printf("Motor detenido.\n");
            }
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

    printf("Modo (0=Polling, 1=IRQ, 2=Combinado): ");
    char buf[4]; fgets(buf, sizeof(buf), stdin);
    modo_medicion = atoi(buf);
    if (modo_medicion < 0 || modo_medicion > 2) modo_medicion = 0;
    printf("Modo seleccionado: %d\n", modo_medicion);

    printf("Comandos disponibles:\nSTART <paso PWM>\nPWM <valor PWM>\nSTOP\n");

    modo_interactivo();

    return 0;
}
