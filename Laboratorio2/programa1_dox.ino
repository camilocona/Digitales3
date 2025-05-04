/**
 * @file motor_encoder_control.ino
 * @brief Control de motor DC con lectura de velocidad mediante encoder rotatorio.
 * 
 * Este programa permite controlar la velocidad y dirección de un motor DC usando un puente H
 * y calcula la velocidad angular (RPM) y lineal (km/h) mediante un encoder conectado a una Raspberry Pi Pico.
 * Se configura vía monitor serie.
 * 
 * @author TuNombre
 * @date 2025-05-04
 */

// ==== Pines del puente H y encoder ====

/**
 * @brief Pin de PWM para controlar la velocidad del motor A.
 */
const int ENA = 0;

/**
 * @brief Pin IN1 del puente H para dirección del motor.
 */
const int IN1 = 1;

/**
 * @brief Pin IN2 del puente H para dirección del motor.
 */
const int IN2 = 2;

/**
 * @brief Pin digital conectado al encoder rotatorio.
 */
const int ENCODER_PIN = 10;


// ==== Parámetros del encoder y rueda ====

/**
 * @brief Contador de pulsos del encoder, se incrementa en interrupciones.
 */
volatile unsigned int pulseCount = 0;

/**
 * @brief Marca de tiempo (ms) de la última medición de RPM.
 */
unsigned long lastRPMCheck = 0;

/**
 * @brief Cantidad de pulsos generados por una vuelta completa del eje.
 */
unsigned int pulsesPerRevolution = 20;

/**
 * @brief Diámetro de la rueda en milímetros.
 */
const float wheelDiameterMM = 25.0;


// ==== Variables de control y visualización ====

/**
 * @brief Ciclo de trabajo (PWM) ingresado por el usuario (0–100).
 */
int dutyCycle = -1;

/**
 * @brief Bandera que indica si la dirección fue definida.
 */
bool directionSet = false;

/**
 * @brief Bandera que indica que el sistema está listo para medir velocidad.
 */
bool ready = false;

/**
 * @brief Revoluciones por minuto calculadas.
 */
float rpm = 0;

/**
 * @brief Velocidad calculada en km/h.
 */
float velocityKmh = 0;


// ==== Funciones ====

/**
 * @brief Rutina de interrupción para contar pulsos del encoder.
 * 
 * Se llama automáticamente en cada flanco de subida del pin del encoder.
 */
void countPulse() {
  pulseCount++;
}

/**
 * @brief Función de inicialización del sistema.
 * 
 * Configura pines, comunicación serial, interrupciones y PWM.
 */
void setup() {
  Serial.begin(115200);
  delay(3000); // Da tiempo para abrir el monitor serie

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(ENCODER_PIN, countPulse, RISING);

  analogWriteFreq(1000);     // Frecuencia del PWM
  analogWriteRange(255);     // Escala del PWM

  Serial.println("Ingrese el duty cycle (0–100):");
}

/**
 * @brief Función principal de ejecución continua.
 * 
 * Espera comandos desde el monitor serie, ajusta velocidad y dirección del motor,
 * y calcula RPM y velocidad cada segundo.
 */
void loop() {
  // Lectura de comandos desde el monitor serie
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    int val = input.toInt();

    // Primer paso: ingreso del duty cycle
    if (dutyCycle == -1) {
      if (val >= 0 && val <= 100) {
        dutyCycle = val;
        analogWrite(ENA, map(dutyCycle, 0, 100, 0, 255));
        Serial.println("Duty recibido.");
        Serial.println("¿Dirección? 'f' (adelante) o 'r' (reversa):");
      } else {
        Serial.println("Duty inválido. Intente de nuevo (0–100):");
      }

    // Segundo paso: ingreso de dirección
    } else if (!directionSet) {
      if (input == "f") {
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        directionSet = true;
        Serial.println("Dirección: adelante.");
        ready = true;
      } else if (input == "r") {
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        directionSet = true;
        Serial.println("Dirección: reversa.");
        ready = true;
      } else {
        Serial.println("Dirección inválida. Use 'f' o 'r':");
      }

    // Paso adicional: cambiar duty después de configuración inicial
    } else {
      if (val >= 0 && val <= 100) {
        dutyCycle = val;
        analogWrite(ENA, map(dutyCycle, 0, 100, 0, 255));
        Serial.print("Duty actualizado a: ");
        Serial.println(dutyCycle);
      } else {
        Serial.println("Duty inválido. Intente de nuevo (0–100):");
      }
    }
  }

  // Cálculo de RPM y velocidad cada segundo
  if (ready) {
    unsigned long currentTime = millis();
    if (currentTime - lastRPMCheck >= 1000) {
      noInterrupts();
      unsigned int count = pulseCount;
      pulseCount = 0;
      interrupts();

      rpm = (count / (float)pulsesPerRevolution) * 60.0;
      velocityKmh = rpm * 3.1416 * (wheelDiameterMM / 1000.0) * 60.0 / 1000.0;

      Serial.print("RPM: ");
      Serial.print(rpm);
      Serial.print(" | Velocidad: ");
      Serial.print(velocityKmh);
      Serial.println(" km/h");

      lastRPMCheck = currentTime;
    }
  }
}
