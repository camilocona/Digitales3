from machine import Pin, PWM
from time import ticks_ms, ticks_diff, sleep_ms
import sys
import select

# Pines
ENA = PWM(Pin(0))
IN1 = Pin(1, Pin.OUT)
IN2 = Pin(2, Pin.OUT)
ENCODER_PIN = Pin(10, Pin.IN, Pin.PULL_UP)

# Parámetros
pulse_count = 0
last_pulse_count = 0
pulses_per_rev = 20

max_samples = 5000
timestamp = []
pwm_buffer = []
rpm_buffer = []
sample_index = 0

last_sample_time = 0
start_time = 0
capturing = False
sistema_activo = True

current_pwm = 0  # PWM aplicado manualmente

# Configurar PWM
ENA.freq(1000)

# Interrupción del encoder
def count_pulse(pin):
    global pulse_count
    pulse_count += 1

ENCODER_PIN.irq(trigger=Pin.IRQ_RISING, handler=count_pulse)

# Dirección fija
IN1.value(1)
IN2.value(0)

print("timestamp_ms,pwm_percent,rpm")

start_time = ticks_ms()
last_send_time = 0

def set_pwm(value):
    global current_pwm
    current_pwm = value
    ENA.duty_u16(int(value * 65535 / 100))
    print("PWM ajustado a:", value)

def send_manual_data():
    global pulse_count, last_pulse_count
    if current_pwm == 0:
        return

    current_count = pulse_count
    delta = current_count - last_pulse_count
    last_pulse_count = current_count

    rpm = (delta / pulses_per_rev) * (60000 / 500)

    print("PWM:", current_pwm, ", RPM:", round(rpm, 2))

def start_capture(incremento_pwm):
    global capturing, sample_index, last_sample_time, last_pulse_count, pulse_count
    capturing = True
    sample_index = 0
    print("Iniciando captura con incremento de PWM:", incremento_pwm)

    step_pwm = [0, incremento_pwm, 2*incremento_pwm, 3*incremento_pwm, 4*incremento_pwm, 5*incremento_pwm,
                4*incremento_pwm, 3*incremento_pwm, 2*incremento_pwm, incremento_pwm, 0]

    for pwm in step_pwm:
        ENA.duty_u16(int(pwm * 65535 / 100))
        step_start = ticks_ms()
        while ticks_diff(ticks_ms(), step_start) < 2000:
            now = ticks_ms()
            if ticks_diff(now, last_sample_time) >= 4:
                current_count = pulse_count
                delta = current_count - last_pulse_count
                last_pulse_count = current_count

                rpm = (delta / pulses_per_rev) * (60000 / 4)

                if sample_index < max_samples:
                    timestamp.append(ticks_diff(now, start_time))
                    pwm_buffer.append(pwm)
                    rpm_buffer.append(rpm)
                    sample_index += 1

                last_sample_time = now

    for i in range(sample_index):
        print(f"{timestamp[i]},{pwm_buffer[i]},{rpm_buffer[i]:.2f}")

    print("Secuencia completada.")
    capturing = False

# Loop principal
while True:
    # Leer comandos
    if sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
        cmd = sys.stdin.readline().strip()

        if cmd.startswith("START"):
            try:
                val = int(cmd[6:])
                if 0 <= val <= 100:
                    start_capture(val)
                else:
                    print("Valor fuera de rango. PWM debe estar entre 0 y 100.")
            except:
                print("Comando START inválido.")

        elif cmd.startswith("PWM"):
            try:
                val = int(cmd[4:])
                if 0 <= val <= 100:
                    set_pwm(val)
                else:
                    print("Valor fuera de rango. PWM debe estar entre 0 y 100.")
            except:
                print("Comando PWM inválido.")

        elif cmd.upper() == "STOP":
            sistema_activo = False
            ENA.duty_u16(0)
            current_pwm = 0
            print("Sistema detenido.")

    # Envío automático si está activo y no capturando
    if sistema_activo and not capturing:
        if ticks_diff(ticks_ms(), last_send_time) >= 500:
            send_manual_data()
            last_send_time = ticks_ms()

    sleep_ms(10)
