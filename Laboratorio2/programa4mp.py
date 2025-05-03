from machine import Pin, PWM
import utime
import select
import sys

# Pines
ENA = PWM(Pin(0))
IN1 = Pin(1, Pin.OUT)
IN2 = Pin(2, Pin.OUT)
ENCODER_PIN = Pin(10, Pin.IN, Pin.PULL_UP)

# Configuración PWM
ENA.freq(1000)
pwm_duty = 0

# Variables de encoder
pulse_count = 0
last_pulse_count = 0
pulses_per_rev = 20

# Buffers
timestamp = []
pwm_buffer = []
rpm_buffer = []
max_samples = 5000
sample_index = 0

# Estados
last_sample_time = 0
start_time = utime.ticks_ms()
capturing = False
sistema_activo = True

# Dirección fija
IN1.value(1)
IN2.value(0)

# Interrupción
def count_pulse(pin):
    global pulse_count
    pulse_count += 1

ENCODER_PIN.irq(trigger=Pin.IRQ_RISING, handler=count_pulse)

# Función para ajustar PWM
def set_pwm(porcentaje):
    global pwm_duty
    porcentaje = max(0, min(porcentaje, 100))  # Limitar entre 0 y 100
    pwm_duty = int(porcentaje * 65535 / 100)
    ENA.duty_u16(pwm_duty)
    print("PWM ajustado a:", porcentaje, "%")

# Función para captura
def start_capture(incremento):
    global sample_index, last_sample_time, capturing, last_pulse_count

    capturing = True
    sample_index = 0
    print("Iniciando captura con incremento de PWM:", incremento)

    # PWM creciente y decreciente, terminando en 0
    subida = list(range(0, 101, incremento))

    # Generar bajada manualmente asegurando que termine en 0
    bajada = []
    actual = 100 - incremento
    while actual > 0:
        bajada.append(actual)
        actual -= incremento
    if bajada[-1] != 0:
        bajada.append(0)
    elif bajada == []:
        bajada = [0]

    step_pwm = subida + bajada

    for pwm in step_pwm:
        set_pwm(pwm)
        step_start = utime.ticks_ms()
        while utime.ticks_diff(utime.ticks_ms(), step_start) < 2000:
            now = utime.ticks_ms()
            if utime.ticks_diff(now, last_sample_time) >= 4:
                current_count = pulse_count
                delta = current_count - last_pulse_count
                last_pulse_count = current_count

                rpm = (delta / pulses_per_rev) * (60000 / 4)

                if sample_index < max_samples:
                    timestamp.append(utime.ticks_diff(now, start_time))
                    pwm_buffer.append(pwm)
                    rpm_buffer.append(rpm)
                    sample_index += 1

                last_sample_time = now

    for i in range(sample_index):
        print("{},{},{}".format(timestamp[i], pwm_buffer[i], rpm_buffer[i]))

    print("Secuencia completada.")
    capturing = False

# Enviar datos manuales periódicamente
def enviar_datos():
    global last_pulse_count
    if pwm_duty == 0:
        return

    current_count = pulse_count
    delta = current_count - last_pulse_count
    last_pulse_count = current_count

    rpm = (delta / pulses_per_rev) * (60000 / 500)
    velocidad_kmh = rpm * 0.002  # Ajusta según rueda/transmisión
    print("RPM: {:.2f} | Velocidad: {:.2f} km/h".format(rpm, velocidad_kmh))

# Loop principal
print("timestamp_ms,pwm_percent,rpm")  # Encabezado CSV

last_print = utime.ticks_ms()
while True:
    # Leer comandos desde la consola
    if select.select([sys.stdin], [], [], 0)[0]:
        comando = sys.stdin.readline().strip()

        if comando.startswith("START"):
            try:
                valor = int(comando[6:])
                if 0 < valor <= 100:
                    start_capture(valor)
                else:
                    print("Valor fuera de rango. Usa entre 1 y 100.")
            except:
                print("Comando START inválido.")

        elif comando.startswith("PWM"):
            try:
                valor = int(comando[4:])
                if 0 <= valor <= 100:
                    set_pwm(valor)
                else:
                    print("Valor fuera de rango. Usa entre 0 y 100.")
            except:
                print("Comando PWM inválido.")

        elif comando.upper() == "STOP":
            sistema_activo = False
            set_pwm(0)
            print("Sistema detenido.")

        elif comando == "":
            pass  # Ignorar línea vacía
        else:
            print("Comando desconocido:", comando)

    # Envío de RPM continuo si no está capturando
    if sistema_activo and not capturing:
        now = utime.ticks_ms()
        if utime.ticks_diff(now, last_print) >= 500:
            enviar_datos()
            last_print = now
