from machine import Pin, PWM
from time import ticks_ms, ticks_diff
import sys

# Pines del motor A (puente H)
ENA = PWM(Pin(0))
IN1 = Pin(1, Pin.OUT)
IN2 = Pin(2, Pin.OUT)

# Encoder en GPIO10
ENCODER_PIN = Pin(10, Pin.IN, Pin.PULL_UP)

# Parámetros
pulse_count = 0
last_pulse_count = 0
pulses_per_rev = 20  # Muescas del encoder

# Buffer de datos
stepPWM = [0, 20, 40, 60, 80, 100, 80, 60, 40, 20, 0]
step_duration = 2000  # Duración de cada paso (2 segundos)
sample_interval = 4  # Intervalo de muestreo (4 ms = 250 Hz)
max_samples = 5000  # Buffer suficientemente grande

timestamp = [0] * max_samples
pwmBuffer = [0] * max_samples
rpmBuffer = [0.0] * max_samples
sample_index = 0

last_sample_time = 0
start_time = 0

# Configuración del PWM
ENA.freq(1000)

# Interrupción para contar pulsos
def count_pulse(pin):
    global pulse_count
    pulse_count += 1

# Configurar la interrupción
ENCODER_PIN.irq(trigger=Pin.IRQ_RISING, handler=count_pulse)

# Configuración inicial
print("timestamp_ms,pwm_percent,rpm")  # Encabezado CSV
start_time = ticks_ms()

# Función principal
while True:
    # Iterar a través de los pasos PWM
    for pwm in stepPWM:
        ENA.duty_u16(int(pwm * 65535 / 100))  # Mapear pwm a 0-65535

        step_start = ticks_ms()
        while ticks_diff(ticks_ms(), step_start) < step_duration:  # 2 segundos por paso
            current_time = ticks_ms()

            if ticks_diff(current_time, last_sample_time) >= sample_interval:
                # Medir diferencia de pulsos
                current_count = pulse_count
                delta_pulses = current_count - last_pulse_count
                last_pulse_count = current_count

                # Calcular RPM basado en delta de pulsos
                rpm = (delta_pulses / pulses_per_rev) * (60000.0 / sample_interval)

                # Guardar en buffer
                if sample_index < max_samples:
                    timestamp[sample_index] = current_time - start_time
                    pwmBuffer[sample_index] = pwm
                    rpmBuffer[sample_index] = rpm
                    sample_index += 1

                last_sample_time = current_time

        # Terminado el ciclo de paso, imprimir los resultados
    for i in range(sample_index):
        print(f"{timestamp[i]},{pwmBuffer[i]},{rpmBuffer[i]:.2f}")

    print("Secuencia completada.")
    break  # Terminar el ciclo
