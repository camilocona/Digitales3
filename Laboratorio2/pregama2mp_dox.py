##
# @file motor_step_response.py
# @brief Prueba de respuesta escalonada para motor DC usando PWM y encoder, en MicroPython.
#
# Este script aplica una serie de pasos de PWM a un motor DC controlado por un puente H y mide su velocidad
# usando un encoder rotatorio. Los datos se almacenan en buffers y se imprimen en formato CSV para análisis posterior.
#
# Diseñado para placas compatibles con MicroPython como Raspberry Pi Pico.
#
# @author TuNombre
# @date 2025-05-04

from machine import Pin, PWM
from time import ticks_ms, ticks_diff
import sys

# === Configuración de pines ===

## @brief PWM del motor (ENA en el puente H)
ENA = PWM(Pin(0))

## @brief Pin IN1 del puente H
IN1 = Pin(1, Pin.OUT)

## @brief Pin IN2 del puente H
IN2 = Pin(2, Pin.OUT)

## @brief Pin del encoder rotatorio (entrada con resistencia pull-up)
ENCODER_PIN = Pin(10, Pin.IN, Pin.PULL_UP)

# === Parámetros de encoder y motor ===

## @brief Contador global de pulsos del encoder (actualizado en interrupción)
pulse_count = 0

## @brief Pulsos registrados en la última muestra
last_pulse_count = 0

## @brief Número de pulsos por vuelta completa del encoder
pulses_per_rev = 20

# === Buffers y configuración del experimento ===

## @brief Secuencia de PWM en forma escalonada (%)
stepPWM = [0, 20, 40, 60, 80, 100, 80, 60, 40, 20, 0]

## @brief Duración de cada paso PWM (en milisegundos)
step_duration = 2000

## @brief Intervalo de muestreo entre datos (en milisegundos)
sample_interval = 4

## @brief Tamaño máximo de los buffers
max_samples = 5000

## @brief Tiempos (ms) en los que se tomó cada muestra, relativo al inicio
timestamp = [0] * max_samples

## @brief PWM aplicado en cada muestra
pwmBuffer = [0] * max_samples

## @brief RPM calculadas en cada muestra
rpmBuffer = [0.0] * max_samples

## @brief Índice actual para insertar datos en los buffers
sample_index = 0

## @brief Última marca de tiempo en la que se hizo una muestra
last_sample_time = 0

## @brief Tiempo de inicio del experimento (en ms)
start_time = 0

# === Configuración del PWM ===

ENA.freq(1000)  # Frecuencia de operación del PWM: 1 kHz

##
# @brief Rutina de interrupción para contar pulsos del encoder.
# Se ejecuta en cada flanco ascendente detectado en ENCODER_PIN.
#
# @param pin Objeto Pin que generó la interrupción (no se usa).
def count_pulse(pin):
    global pulse_count
    pulse_count += 1

# === Configuración de la interrupción del encoder ===

ENCODER_PIN.irq(trigger=Pin.IRQ_RISING, handler=count_pulse)

# === Inicialización y encabezado de salida ===

print("timestamp_ms,pwm_percent,rpm")  # Encabezado en formato CSV
start_time = ticks_ms()  # Marca de tiempo del inicio del experimento

# === Función principal ===

while True:
    # Recorre la secuencia de PWM paso a paso
    for pwm in stepPWM:
        ENA.duty_u16(int(pwm * 65535 / 100))  # Convierte porcentaje a rango 16 bits (0–65535)

        step_start = ticks_ms()
        while ticks_diff(ticks_ms(), step_start) < step_duration:
            current_time = ticks_ms()

            # Verifica si es momento de tomar una nueva muestra
            if ticks_diff(current_time, last_sample_time) >= sample_interval:
                current_count = pulse_count
                delta_pulses = current_count - last_pulse_count
                last_pulse_count = current_count

                # Calcula RPM: (pulsos / pulsos por vuelta) * (60000 ms / intervalo)
                rpm = (delta_pulses / pulses_per_rev) * (60000.0 / sample_interval)

                # Almacena la muestra si hay espacio
                if sample_index < max_samples:
                    timestamp[sample_index] = current_time - start_time
                    pwmBuffer[sample_index] = pwm
                    rpmBuffer[sample_index] = rpm
                    sample_index += 1

                last_sample_time = current_time

    # Finalizada la secuencia: imprimir todos los datos
    for i in range(sample_index):
        print(f"{timestamp[i]},{pwmBuffer[i]},{rpmBuffer[i]:.2f}")

    print("Secuencia completada.")
    break  # Termina la ejecución después de una corrida completa
