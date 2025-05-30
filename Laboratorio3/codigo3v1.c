#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"

// === Configuración ===
#define PIN_ENCODER 2
#define TICK_MS 100 // cada cuanto calcular la RPM (100 ms)
#define PULSOS_POR_REV 16
#define MAX_ESCALONES 3000

// === Variables globales ===
volatile uint32_t contador_pulsos = 0; // cuenta los pulsos desde la ultima medicion
uint32_t escalones[MAX_ESCALONES];
uint32_t rpm_medida[MAX_ESCALONES];
uint32_t pwm_aplicado[MAX_ESCALONES];
volatile uint32_t ciclos = 0;
bool grabando = false;
bool ultimaGrabando = false;

// === Interrupcion por flanco de subida ===
void encoder_callback(uint gpio, uint32_t events) {
    if (grabando) {
        contador_pulsos++;
    }
}

// === Funcion para calcular RPM cada TICK_MS ===
bool calcular_rpm(struct repeating_timer *t) {
    if (grabando) {
        // Calcular RPM a partir del numero de pulsos
        float rpm = (contador_pulsos * 600.0f) / PULSOS_POR_REV; // porque TICK_MS=100ms
        if (ciclos < MAX_ESCALONES && rpm > 0.0f) {
            escalones[ciclos] = to_us_since_boot(get_absolute_time()) / 1000; // timestamp ms
            pwm_aplicado[ciclos] = pwm_get_gpio_level(0); // PWM actual
            rpm_medida[ciclos] = (uint32_t)rpm;
            ciclos++;
        }
        contador_pulsos = 0; // reiniciar para siguiente ventana
    }
    return true; // repetir
}

int main() {
    stdio_init_all();

    // Configuracion encoder
    gpio_set_function(PIN_ENCODER, GPIO_FUNC_SIO);
    gpio_set_dir(PIN_ENCODER, GPIO_IN);
    gpio_pull_up(PIN_ENCODER);
    gpio_set_irq_enabled_with_callback(PIN_ENCODER, GPIO_IRQ_EDGE_RISE, true, &encoder_callback);

    // Configuracion PWM en GP0
    gpio_set_function(0, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(0);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.f);
    pwm_config_set_wrap(&config, 62500); // periodo 5ms para 200Hz
    pwm_init(slice, &config, true);

    // Timer repetitivo para calcular RPM cada TICK_MS
    struct repeating_timer timer;
    add_repeating_timer_ms(TICK_MS, calcular_rpm, NULL, &timer);

    printf("Inicio de prueba...\n");

    // Bucle de barrido PWM
    for (int duty = 20; duty <= 100; duty += 20) {
        pwm_set_gpio_level(0, duty * 625); // aplicar PWM
        grabando = true;
        sleep_ms(5000); // mantener 5s
        grabando = false;
        sleep_ms(500); // espera para cambio de escalon
    }

    // Apagar PWM
    pwm_set_gpio_level(0, 0);

    // Guardar archivo
    FILE *f = fopen("rpm_combinado.csv", "w");
    if (f == NULL) {
        printf("Error al abrir el archivo!\n");
        return 1;
    }

    for (uint32_t i = 0; i < ciclos; i++) {
        fprintf(f, "%u,%u,%.2f\n", escalones[i], pwm_aplicado[i],
                (float)rpm_medida[i]);
    }

    fclose(f);
    printf("Datos guardados en rpm_combinado.csv\n");

    return 0;
}
