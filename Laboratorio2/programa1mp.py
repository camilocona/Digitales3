from machine import Pin, PWM
from time import ticks_ms, ticks_diff, sleep_ms
import sys
import select

# Pines del motor A (puente H)
ENA = PWM(Pin(0))
IN1 = Pin(1, Pin.OUT)
IN2 = Pin(2, Pin.OUT)

# Encoder en GPIO10
ENCODER_PIN = Pin(10, Pin.IN, Pin.PULL_UP)

# Parámetros
pulse_count = 0
last_rpm_check = ticks_ms()
pulses_per_revolution = 20
wheel_diameter_mm = 25.0

# Control
duty_cycle = -1
direction_set = False
ready = False
rpm = 0
velocity_kmh = 0

# Configuración del PWM
ENA.freq(1000)

# Interrupción para contar pulsos
def count_pulse(pin):
    global pulse_count
    pulse_count += 1

ENCODER_PIN.irq(trigger=Pin.IRQ_RISING, handler=count_pulse)

# Instrucciones iniciales
print("Ingrese el duty cycle (0–100):")

while True:
    # Revisar entrada sin bloquear
    if sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
        input_line = sys.stdin.readline().strip()

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

    # Cálculo constante de RPM y velocidad
    current_time = ticks_ms()
    if ready and ticks_diff(current_time, last_rpm_check) >= 1000:
        count = pulse_count
        pulse_count = 0

        rpm = (count / pulses_per_revolution) * 60.0
        velocity_kmh = rpm * 3.1416 * (wheel_diameter_mm / 1000.0) * 60.0 / 1000.0

        print("RPM: {:.2f} | Velocidad: {:.2f} km/h".format(rpm, velocity_kmh))
        last_rpm_check = current_time

    sleep_ms(10)  # Pequeño retraso para no saturar la CPU
