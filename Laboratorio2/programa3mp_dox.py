##
# @file programa3mp_dox.py
# @brief Control de motor con PWM y medición de velocidad usando encoder.

# Este código utiliza un PWM para controlar la velocidad de un motor DC y un encoder para medir su velocidad de rotación en RPM.
# Se incluyen funciones para capturar datos a diferentes niveles de PWM, controlar manualmente el PWM y visualizar datos de velocidad.

# @author Maria Valentina Quiroga Alzate, Camilo Andrés Anacona Anacona
# @date 2025-05-04

from machine import Pin, PWM
import time
import sys
import math
import select

# Pines
ENA = PWM(Pin(0))  ///< Pin de control de habilitación del motor (PWM).
IN1 = Pin(1, Pin.OUT)  ///< Pin de control de dirección (entrada 1 del puente H).
IN2 = Pin(2, Pin.OUT)  ///< Pin de control de dirección (entrada 2 del puente H).
ENCODER_PIN = Pin(10, Pin.IN, Pin.PULL_UP)  ///< Pin conectado al encoder rotatorio.

# Configuración PWM
ENA.freq(1000)  ///< Establece la frecuencia del PWM a 1 kHz.
ENA.duty_u16(0)  ///< Inicializa el ciclo de trabajo del PWM a 0.
IN1.value(1)  ///< Dirección fija para el motor (sentido horario).
IN2.value(0)  ///< Dirección fija para el motor (sentido horario).

# Variables
pulsos = 0  ///< Contador de pulsos del encoder.
ultimo_pulsos = 0  ///< Valor del contador de pulsos en la última medición.
pulsos_por_rev = 20  ///< Número de pulsos por vuelta del encoder.
sistema_activo = True  ///< Flag que indica si el sistema está activo.
capturando = False  ///< Flag que indica si se está capturando datos.
pwm_actual = 0  ///< Valor actual de PWM manualmente ajustado.
ultimo_envio = time.ticks_ms()  ///< Última marca de tiempo de la medición de datos.

# Interrupción
def contar_pulsos(pin):
    """
    Función de interrupción que se ejecuta cada vez que el encoder genera un pulso.

    Esta función es llamada cada vez que el pin ENCODER_PIN detecta un flanco ascendente.
    Incrementa el contador global de pulsos del encoder.
    """
    global pulsos
    pulsos += 1

ENCODER_PIN.irq(trigger=Pin.IRQ_RISING, handler=contar_pulsos)  # Configura la interrupción en el pin del encoder.

# Función para establecer PWM
def set_pwm(porcentaje):
    """
    Ajusta el valor del PWM del motor.

    Esta función recibe un porcentaje de PWM (entre 0 y 100) y ajusta el ciclo de trabajo del PWM
    correspondiente al valor deseado.

    @param porcentaje Valor de PWM entre 0 y 100.
    """
    global pwm_actual
    pwm_actual = porcentaje
    duty = int(porcentaje * 65535 / 100)  # Calcula el ciclo de trabajo en un rango de 0 a 65535.
    ENA.duty_u16(duty)  # Establece el ciclo de trabajo del PWM.
    print("PWM ajustado a: {} %".format(porcentaje))

# Función para calcular RPM
def calcular_rpm(delta_pulsos, periodo_ms):
    """
    Calcula las revoluciones por minuto (RPM) a partir del número de pulsos y el periodo en milisegundos.

    @param delta_pulsos Número de pulsos generados por el encoder durante el periodo.
    @param periodo_ms Periodo en milisegundos durante el cual se capturaron los pulsos.
    @return RPM calculada.
    """
    return (delta_pulsos / pulsos_por_rev) * (60000 / periodo_ms)  # Calcula las RPM.

# Función para calcular velocidad
def calcular_velocidad(rpm):
    """
    Calcula la velocidad lineal en km/h a partir de las RPM del motor.

    @param rpm Revoluciones por minuto calculadas del motor.
    @return Velocidad en km/h.
    """
    radio_rueda_m = 0.03  # Radio de la rueda en metros (3 cm).
    velocidad_mps = (2 * math.pi * radio_rueda_m) * (rpm / 60)  # Velocidad en metros por segundo.
    return velocidad_mps * 3.6  # Convierte a km/h.

# Función de captura automática
def start_capture(incremento_pwm):
    """
    Inicia la captura de datos del motor variando el PWM.

    La función aplica diferentes niveles de PWM y captura la cantidad de pulsos del encoder
    durante un intervalo de tiempo. Luego, calcula y muestra las RPM y la velocidad.

    @param incremento_pwm Incremento del valor de PWM (en porcentaje).
    """
    global capturando, pulsos, ultimo_pulsos, ultimo_envio

    if incremento_pwm <= 0 or incremento_pwm > 100:
        print("Valor fuera de rango. Usa entre 1 y 100.")
        return

    capturando = True
    print("Iniciando captura con incremento de PWM: {}".format(incremento_pwm))

    # Secuencia de PWM: incremento y luego decremento
    secuencia_pwm = [i for i in range(0, 101, incremento_pwm)] + \
                    [i for i in range(100 - incremento_pwm, -1, -incremento_pwm)]

    for pwm in secuencia_pwm:
        set_pwm(pwm)  # Ajusta el PWM
        tiempo_inicio = time.ticks_ms()  # Marca de tiempo del inicio de la secuencia.

        # Captura durante 2 segundos por cada nivel de PWM.
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

    set_pwm(0)  # Detiene el motor.
    capturando = False
    print("Secuencia completada.")

# Bucle principal
print("Listo para recibir comandos: START <valor>, PWM <valor>, STOP")

while True:
    # Espera comandos desde la entrada estándar (terminal).
    if sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
        comando = sys.stdin.readline().strip().upper()

        if comando.startswith("START"):
            partes = comando.split()
            if len(partes) == 2 and partes[1].isdigit():
                valor = int(partes[1])
                start_capture(valor)  # Inicia la captura con el valor de PWM proporcionado.
            else:
                print("Comando START inválido. Usa: START <valor entre 1 y 100>")

        elif comando.startswith("PWM"):
            partes = comando.split()
            if len(partes) == 2 and partes[1].isdigit():
                valor = int(partes[1])
                if 0 <= valor <= 100:
                    set_pwm(valor)  # Ajusta el valor del PWM.
                else:
                    print("Valor fuera de rango (0–100)")
            else:
                print("Comando PWM inválido. Usa: PWM <valor entre 0 y 100>")

        elif comando == "STOP":
            set_pwm(0)  # Detiene el motor.
            sistema_activo = False  # Desactiva el sistema.
            print("Sistema detenido.")

        else:
            print("Comando desconocido.")

    # Si el sistema está activo y no se está capturando, se muestra la información de velocidad cada 500 ms.
    if sistema_activo and not capturando:
        if time.ticks_diff(time.ticks_ms(), ultimo_envio) >= 500:
            delta = pulsos - ultimo_pulsos
            ultimo_pulsos = pulsos
            rpm = calcular_rpm(delta, 500)
            velocidad = calcular_velocidad(rpm)
            if not (pwm_actual == 0 and rpm == 0 and velocidad == 0):
                print("PWM: {} %, RPM: {:.2f} | Velocidad: {:.2f} km/h".format(pwm_actual, rpm, velocidad))
            ultimo_envio = time.ticks_ms()
