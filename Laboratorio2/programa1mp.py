from machine import Pin, PWM
from time import ticks_ms, ticks_diff, sleep

# Pines
ENA = PWM(Pin(0), freq=1000)  # Se crea PWM con frecuencia, sin duty inicial
IN1 = Pin(1, Pin.OUT)
IN2 = Pin(2, Pin.OUT)
ENCODER_PIN = Pin(10, Pin.IN, Pin.PULL_UP)

# Parámetros
pulse_count = 0
last_rpm_check = ticks_ms()
pulses_per_rev = 20  # Asegúrate de que este número sea correcto
wheel_diameter_mm = 25.0

# Estado
duty_cycle = -1
direction_set = False
ready = False

# Interrupción del encoder
def count_pulse(pin):
    global pulse_count
    pulse_count += 1
    #print("¡Pulsos incrementados! Pulse count:", pulse_count)  # Depuración: Muestra pulsos

ENCODER_PIN.irq(trigger=Pin.IRQ_RISING, handler=count_pulse)

# Función para mapear rango como en Arduino
def map_range(x, in_min, in_max, out_min, out_max):
    return int((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min)

print("Ingrese el duty cycle (0–100):")

while True:
    # Esperamos a que el usuario ingrese un valor
    input_str = input().strip()  # Usamos input() para leer la entrada

    if duty_cycle == -1:
        try:
            val = int(input_str)
            if 0 <= val <= 100:
                duty_cycle = val
                pwm_val = map_range(duty_cycle, 0, 100, 0, 65535)  # Rango de 0 a 65535
                ENA.duty_u16(pwm_val)  # Se ajusta el duty aquí
                print("Duty recibido.")
                print("¿Dirección? 'f' (adelante) o 'r' (reversa):")
            else:
                print("Duty inválido. Intente de nuevo (0–100):")
        except ValueError:
            print("Entrada inválida. Intente de nuevo.")
    elif not direction_set:
        if input_str == 'f':
            IN1.on()
            IN2.off()
            direction_set = True
            ready = True
            print("Dirección: adelante.")
        elif input_str == 'r':
            IN1.off()
            IN2.on()
            direction_set = True
            ready = True
            print("Dirección: reversa.")
        else:
            print("Dirección inválida. Use 'f' o 'r':")
    else:
        try:
            val = int(input_str)
            if 0 <= val <= 100:
                duty_cycle = val
                pwm_val = map_range(duty_cycle, 0, 100, 0, 65535)  # Rango de 0 a 65535
                ENA.duty_u16(pwm_val)  # Actualización de duty
                print("Duty actualizado a:", duty_cycle)
            else:
                print("Duty inválido. Intente de nuevo (0–100):")
        except ValueError:
            print("Entrada inválida.")

    # Medición continua de RPM y velocidad
    if ready:
        # Imprimir pulsos contados en cada ciclo del loop
        print("Pulsos contados:", pulse_count)

        count = pulse_count
        pulse_count = 0  # Reiniciar el contador después de medir

        rpm = (count / pulses_per_rev) * 60.0  # Calcular RPM
        velocity_kmh = rpm * 3.1416 * (wheel_diameter_mm / 1000.0) * 60.0 / 1000.0  # Calcular velocidad en km/h

        # Imprimir los resultados de RPM y velocidad en cada ciclo
        print("RPM:", round(rpm, 2), "| Velocidad:", round(velocity_kmh, 2), "km/h")

    sleep(0.1)  # Pausa pequeña
