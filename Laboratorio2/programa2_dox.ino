/**
 * @file programa2_dox.ino
 * @brief Control y adquisición de datos para un motor DC con encoder, usando una secuencia escalonada de PWM.
 *
 * Este programa genera una serie de señales PWM en forma de escalón para un motor DC, mide su velocidad (RPM)
 * mediante un encoder rotatorio, y guarda los datos de tiempo, PWM aplicado y RPM resultante en buffers.
 * Finalmente, imprime todos los datos en formato CSV por el puerto serial.
 *
 * Aplicación útil para caracterizar dinámicamente motores (respuesta a entrada escalón).
 *
 * @author Maria Valentina Quiroga Alzate, Camilo Andrés Anacona Anacona
 * @date 2025-05-04
 */

// === Pines utilizados ===

/**
 * @brief Pin GPIO que controla la velocidad del motor mediante PWM (ENA del puente H).
 */
const int ENA = 0;

/**
 * @brief Pin GPIO que controla la dirección IN1 del puente H.
 */
const int IN1 = 1;

/**
 * @brief Pin GPIO que controla la dirección IN2 del puente H.
 */
const int IN2 = 2;

/**
 * @brief Pin GPIO que recibe la señal digital del encoder rotatorio.
 */
const int ENCODER_PIN = 10;


// === Variables para el conteo de pulsos del encoder ===

/**
 * @brief Número total de pulsos detectados desde el inicio. Se incrementa en la interrupción.
 */
volatile unsigned int pulseCount = 0;

/**
 * @brief Número de pulsos registrado en la última muestra. Se usa para calcular la diferencia.
 */
unsigned int lastPulseCount = 0;

/**
 * @brief Número de pulsos generados por una revolución completa del eje del motor.
 */
unsigned int pulsesPerRev = 20;


// === Parámetros de la prueba escalonada ===

/**
 * @brief Arreglo con los valores de PWM (en %) que se aplicarán secuencialmente al motor.
 */
const int stepPWM[] = {0, 20, 40, 60, 80, 100, 80, 60, 40, 20, 0};

/**
 * @brief Tiempo (en ms) durante el cual se mantiene cada valor de PWM.
 */
const int stepDuration = 2000;

/**
 * @brief Intervalo de tiempo (en ms) entre cada muestra de RPM. 4 ms equivale a 250 Hz de muestreo.
 */
const int sampleInterval = 4;


// === Buffers para almacenamiento temporal de datos ===

/**
 * @brief Número máximo de muestras que se pueden almacenar.
 */
const int maxSamples = 5000;

/**
 * @brief Tiempos (en milisegundos desde el inicio) en los que se registró cada muestra.
 */
unsigned long timestamp[maxSamples];

/**
 * @brief PWM aplicado (en %) correspondiente a cada muestra.
 */
int pwmBuffer[maxSamples];

/**
 * @brief RPM medidas en cada instante de muestreo.
 */
float rpmBuffer[maxSamples];

/**
 * @brief Índice actual en los buffers de datos.
 */
int sampleIndex = 0;


// === Variables de tiempo para control de muestreo ===

/**
 * @brief Marca de tiempo de la última muestra tomada (para respetar el intervalo de muestreo).
 */
unsigned long lastSampleTime = 0;

/**
 * @brief Tiempo de inicio del experimento (usado para calcular tiempos relativos).
 */
unsigned long startTime = 0;


/**
 * @brief Función de interrupción que incrementa el contador de pulsos cuando se detecta un flanco ascendente.
 *
 * Esta función se asocia al pin del encoder mediante `attachInterrupt`.
 */
void countPulse() {
  pulseCount++;
}


/**
 * @brief Configuración inicial del sistema. Define modos de los pines, inicializa serial, y configura PWM.
 */
void setup() {
  Serial.begin(115200);            ///< Inicializa la comunicación serie a 115200 baudios.
  delay(2000);                     ///< Espera para permitir que el monitor serie se conecte.
  for (int i = 0; i < 10; i++) Serial.println();  ///< Limpia consola con líneas en blanco.

  // Configura los pines del puente H y del encoder
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENCODER_PIN, INPUT_PULLUP);  ///< Entrada con resistencia de pull-up para evitar flotación.

  // Configura interrupción para contar pulsos del encoder
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), countPulse, RISING);

  // Configura la frecuencia y rango del PWM
  analogWriteFreq(1000);       ///< Frecuencia del PWM: 1 kHz.
  analogWriteRange(255);       ///< Rango de valores del PWM: 0–255 (Arduino style).

  // Fija dirección del motor (adelante)
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  // Guarda tiempo inicial
  startTime = millis();

  // Imprime encabezado de la tabla CSV
  Serial.println("timestamp_ms,pwm_percent,rpm");
}


/**
 * @brief Bucle principal del programa. Ejecuta cada paso PWM, mide RPM periódicamente, y guarda resultados.
 */
void loop() {
  // Recorre cada valor PWM definido en la secuencia
  for (int i = 0; i < sizeof(stepPWM) / sizeof(stepPWM[0]); i++) {
    int pwm = stepPWM[i];
    analogWrite(ENA, map(pwm, 0, 100, 0, 255));  ///< Convierte porcentaje a valor de 0–255 y lo aplica

    // Mantiene el PWM constante durante 'stepDuration' milisegundos
    unsigned long stepStart = millis();
    while (millis() - stepStart < stepDuration) {
      unsigned long currentTime = millis();

      // Si ya ha pasado 'sampleInterval', se toma una nueva muestra
      if (currentTime - lastSampleTime >= sampleInterval) {
        // Protege lectura del contador en entorno con interrupciones
        noInterrupts();
        unsigned int currentCount = pulseCount;
        interrupts();

        // Calcula la diferencia de pulsos desde la última muestra
        unsigned int deltaPulses = currentCount - lastPulseCount;
        lastPulseCount = currentCount;

        // Calcula RPM: (pulsos / pulsos por vuelta) * (60000 ms/min / intervalo)
        float rpm = (deltaPulses / (float)pulsesPerRev) * (60000.0 / sampleInterval);

        // Guarda muestra si hay espacio
        if (sampleIndex < maxSamples) {
          timestamp[sampleIndex] = currentTime - startTime;  ///< Tiempo relativo
          pwmBuffer[sampleIndex] = pwm;
          rpmBuffer[sampleIndex] = rpm;
          sampleIndex++;
        }

        lastSampleTime = currentTime;
      }
    }
  }

  // Una vez finalizada la secuencia, imprime los datos recopilados en formato CSV
  for (int i = 0; i < sampleIndex; i++) {
    Serial.print(timestamp[i]);
    Serial.print(",");
    Serial.print(pwmBuffer[i]);
    Serial.print(",");
    Serial.println(rpmBuffer[i]);
  }

  Serial.println("Secuencia completada.");

  // Detiene el programa indefinidamente
  while (true);
}
