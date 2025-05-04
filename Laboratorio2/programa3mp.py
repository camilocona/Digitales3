from machine import Pin, PWM
import time
import sys
import math
import select

# Pines
ENA = PWM(Pin(0))
IN1 = Pin(1, Pin.OUT)
IN2 = Pin(2, Pin.OUT)
ENCODER_PIN = Pin(10, Pin.IN, Pin.PULL_UP)

# Configuración PWM
ENA.freq(1000)
ENA.duty_u16(0)
IN1.value(1)
IN2.value(0)

# Variables
pulsos = 0
ultimo_pulsos = 0
pulsos_por_rev = 20
sistema_activo = True
capturando = False
pwm_actual = 0
ultimo_envio = time.ticks_ms()

# Interrupción
def contar_pulsos(pin):
    global pulsos
    pulsos += 1

ENCODER_PIN.irq(trigger=Pin.IRQ_RISING, handler=contar_pulsos)

# Función para establecer PWM
def set_pwm(porcentaje):
    global pwm_actual
    pwm_actual = porcentaje
    duty = int(porcentaje * 65535 / 100)
    ENA.duty_u16(duty)
    print("PWM ajustado a: {} %".format(porcentaje))

# Función para calcular RPM
def calcular_rpm(delta_pulsos, periodo_ms):
    return (delta_pulsos / pulsos_por_rev) * (60000 / periodo_ms)

# Función para velocidad (ejemplo fijo)
def calcular_velocidad(rpm):
    radio_rueda_m = 0.03  # 3 cm
    velocidad_mps = (2 * math.pi * radio_rueda_m) * (rpm / 60)
    return velocidad_mps * 3.6

# Función de captura automática
def start_capture(incremento_pwm):
    global capturando, pulsos, ultimo_pulsos, ultimo_envio

    if incremento_pwm <= 0 or incremento_pwm > 100:
        print("Valor fuera de rango. Usa entre 1 y 100.")
        return

    capturando = True
    print("Iniciando captura con incremento de PWM: {}".format(incremento_pwm))

    secuencia_pwm = [i for i in range(0, 101, incremento_pwm)] + \
                    [i for i in range(100 - incremento_pwm, -1, -incremento_pwm)]

    for pwm in secuencia_pwm:
        set_pwm(pwm)
        tiempo_inicio = time.ticks_ms()
        while time.ticks_diff(time.ticks_ms(), tiempo_inicio) < 2000:
            if time.ticks_diff(time.ticks_ms(), ultimo_envio) >= 500:
                delta = pulsos - ultimo_pulsos
                ultimo_pulsos = pulsos
                rpm = calcular_rpm(delta, 500)
                velocidad = calcular_velocidad(rpm)
                global ultimo_envio
                if not (pwm == 0 and rpm == 0 and velocidad == 0):
                    print("PWM: {} %, RPM: {:.2f} | Velocidad: {:.2f} km/h".format(pwm, rpm, velocidad))
                ultimo_envio = time.ticks_ms()

    set_pwm(0)
    capturando = False
    print("Secuencia completada.")

# Bucle principal
print("Listo para recibir comandos: START <valor>, PWM <valor>, STOP")

while True:
    if sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
        comando = sys.stdin.readline().strip().upper()

        if comando.startswith("START"):
            partes = comando.split()
            if len(partes) == 2 and partes[1].isdigit():
                valor = int(partes[1])
                start_capture(valor)
            else:
                print("Comando START inválido. Usa: START <valor entre 1 y 100>")

        elif comando.startswith("PWM"):
            partes = comando.split()
            if len(partes) == 2 and partes[1].isdigit():
                valor = int(partes[1])
                if 0 <= valor <= 100:
                    set_pwm(valor)
                else:
                    print("Valor fuera de rango (0–100)")
            else:
                print("Comando PWM inválido. Usa: PWM <valor entre 0 y 100>")

        elif comando == "STOP":
            set_pwm(0)
            sistema_activo = False
            print("Sistema detenido.")

        else:
            print("Comando desconocido.")

    if sistema_activo and not capturando:
        if time.ticks_diff(time.ticks_ms(), ultimo_envio) >= 500:
            delta = pulsos - ultimo_pulsos
            ultimo_pulsos = pulsos
            rpm = calcular_rpm(delta, 500)
            velocidad = calcular_velocidad(rpm)
            if not (pwm_actual == 0 and rpm == 0 and velocidad == 0):
                print("PWM: {} %, RPM: {:.2f} | Velocidad: {:.2f} km/h".format(pwm_actual, rpm, velocidad))
            ultimo_envio = time.ticks_ms()
