##
# @file programa1mp_dox.py
# @brief Control de un motor DC con lectura de velocidad usando encoder rotatorio en Raspberry Pi Pico con MicroPython.
#
# Este script permite al usuario controlar el motor mediante PWM, seleccionar dirección (adelante o reversa),
# y calcular la velocidad de giro (RPM) y velocidad lineal (km/h) usando un encoder conectado.
# La configuración y control se hace a través de la entrada estándar (monitor serial).
#
# @author Maria Valentina Quiroga Alzate, Camilo Andrés Anacona Anacona
# @date 2025-05-04

from machine import Pin, PWM
from time import ticks_ms, ticks_diff, sleep_ms
import sys
import select

# ==== Pines del puente H y encoder ====

## @brief PWM para controlar velocidad del motor (ENA en GPIO0)
ENA = PWM(Pin(0))

## @brief Pin de dirección IN1 del puente H
IN1 = Pin(1, Pin.OUT)

## @brief Pin de dirección IN2 del puente H
IN2 = Pin(2, Pin.OUT)

## @brief Pin de entrada del encoder rotatorio (GPIO10)
ENCODER_PIN = Pin(10, Pin.IN, Pin.PULL_UP)

# ==== Parámetros físicos y de medición ====

## @brief Contador de pulsos del encoder (actualizado por interrupción)
pulse_count = 0

## @brief Marca de tiempo de la última verificación de RPM
last_rpm_check = ticks_ms()

## @brief Número de pulsos por revolución del encoder
pulses_per_revolution = 20

## @brief Diámetro de la rueda en milímetros
wheel_diameter_mm = 25.0

# ==== Variables de control del motor y estado ====

## @brief Ciclo de trabajo ingresado por el usuario (0–100)
duty_cycle = -1

## @brief Indica si la dirección ya fue configurada
direction_set = False

## @brief Bandera de sistema listo para calcular velocidad
ready = False

## @brief Revoluciones por minuto calculadas
rpm = 0

## @brief Velocidad en km/h calculada
velocity_kmh = 0

# ==== Configuración de PWM ====

ENA.freq(1000)  # @brief Se fija la frecuencia del PWM a 1 kHz

##
# @brief Rutina de interrupción para contar pulsos del encoder.
# @param pin Objeto Pin que activó la interrupción.
def count_pulse(pin):
    global pulse_count
    pulse_count += 1

# Asignación de la interrupción
ENCODER_PIN.irq(trigger=Pin.IRQ_RISING, handler=count_pulse)

# ==== Interfaz de usuario ====
print("Ingrese el duty cycle (0–100):")

# ==== Bucle principal de control ====
while True:
    # @brief Revisión de entrada serial no bloqueante
    if sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
        input_line = sys.stdin.readline().strip()

        # Ingreso inicial de ciclo de trabajo
        if duty_cycle == -1:
            try:
                val = int(input_line)
                if 0 <= val <= 100:
                    duty_cycle = val
                    ENA.duty_u16(int(val * 65535 / 100))
                    print("Duty recibido.")
                    print("¿Dirección? 'f' (adelante) o 'r' (reversa):")
                else:
                    print("Duty inválido. Intente de nuevo (0–100):")
            except:
                print("Entrada inválida.")

        # Ingreso de dirección del motor
        elif not direction_set:
            if input_line == "f":
                IN1.value(1)
                IN2.value(0)
                direction_set = True
                ready = True
                print("Dirección: adelante.")
            elif input_line == "r":
                IN1.value(0)
                IN2.value(1)
                direction_set = True
                ready = True
                print("Dirección: reversa.")
            else:
                print("Dirección inválida. Use 'f' o 'r':")

        # Cambio posterior de duty cycle
        else:
            try:
                val = int(input_line)
                if 0 <= val <= 100:
                    duty_cycle = val
                    ENA.duty_u16(int(val * 65535 / 100))
                    print("Duty actualizado a:", duty_cycle)
                else:
                    print("Duty inválido. Intente de nuevo (0–100):")
            except:
                print("Entrada inválida.")

    # @brief Cálculo de RPM y velocidad cada segundo
    current_time = ticks_ms()
    if ready and ticks_diff(current_time, last_rpm_check) >= 1000:
        count = pulse_count
        pulse_count = 0

        rpm = (count / pulses_per_revolution) * 60.0
        velocity_kmh = rpm * 3.1416 * (wheel_diameter_mm / 1000.0) * 60.0 / 1000.0

        print("RPM: {:.2f} | Velocidad: {:.2f} km/h".format(rpm, velocity_kmh))
        last_rpm_check = current_time

    sleep_ms(10)  # @brief Pequeño retardo para liberar CPU
