/**
 * @file programa3_dox.ino
 * @brief Control de motor con PWM y medición de velocidad usando encoder.
 * 
 * Este código utiliza un PWM para controlar la velocidad de un motor DC y un encoder para medir su velocidad de rotación en RPM.
 * Se incluyen funciones para capturar datos a diferentes niveles de PWM, controlar manualmente el PWM y visualizar datos de velocidad.
 * 
 * @author Maria Valentina Quiroga Alzate, Camilo Andrés Anacona Anacona
 * @date 2025-05-04
 */

const int ENA = 0;  ///< Pin de control de habilitación del motor (PWM).
const int IN1 = 1;  ///< Pin de control de dirección (entrada 1 del puente H).
const int IN2 = 2;  ///< Pin de control de dirección (entrada 2 del puente H).
const int ENCODER_PIN = 10;  ///< Pin conectado al encoder rotatorio.

volatile unsigned int pulseCount = 0;  ///< Contador de pulsos del encoder, incrementado en la interrupción.
unsigned int lastPulseCount = 0;  ///< Valor del contador de pulsos en la última medición.
unsigned int pulsesPerRev = 20;  ///< Número de pulsos por vuelta del encoder.

const int maxSamples = 5000;  ///< Número máximo de muestras que pueden ser almacenadas.
unsigned long timestamp[maxSamples];  ///< Array para almacenar las marcas de tiempo de cada muestra.
int pwmBuffer[maxSamples];  ///< Array para almacenar los valores de PWM utilizados en cada muestra.
float rpmBuffer[maxSamples];  ///< Array para almacenar los valores de RPM calculados en cada muestra.
int sampleIndex = 0;  ///< Índice actual para insertar datos en los buffers.

unsigned long lastSampleTime = 0;  ///< Última marca de tiempo de una muestra tomada.
unsigned long startTime = 0;  ///< Marca de tiempo del inicio del experimento.

bool capturing = false;  ///< Flag que indica si la captura de datos está en curso.
bool sistemaActivo = true;  ///< Flag que indica si el sistema está activo.

int currentPWM = 0;  ///< Valor actual de PWM manualmente ajustado.

/**
 * @brief Función de interrupción que se ejecuta cada vez que el encoder genera un pulso.
 * 
 * Esta función es llamada cada vez que el pin ENCODER_PIN detecta un flanco ascendente.
 * Incrementa el contador global de pulsos del encoder.
 */
void countPulse() {
  pulseCount++;
}

/**
 * @brief Configuración inicial de los pines y parámetros de PWM.
 * 
 * Esta función se ejecuta una vez al inicio del programa. Configura los pines de control
 * del motor, establece las interrupciones para contar los pulsos del encoder y configura
 * la frecuencia del PWM.
 */
void setup() {
  Serial.begin(115200);  ///< Inicializa la comunicación serial a 115200 bps.
  delay(2000);  ///< Espera 2 segundos para asegurar la inicialización.
  
  // Configuración de pines
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENCODER_PIN, INPUT_PULLUP);
  
  // Configuración de interrupciones para contar los pulsos del encoder
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), countPulse, RISING);
  
  analogWriteFreq(1000);  ///< Configura la frecuencia del PWM a 1 kHz.
  analogWriteRange(255);  ///< Configura el rango de salida de PWM (0-255).
  
  // Dirección fija para el motor (sentido horario)
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  startTime = millis();  ///< Guarda el tiempo de inicio del programa.
  
  Serial.println("timestamp_ms,pwm_percent,rpm");  ///< Encabezado CSV para los datos a enviar.
}

/**
 * @brief Bucle principal del programa.
 * 
 * En cada iteración, el sistema espera recibir comandos desde el monitor serial. Los comandos
 * permiten iniciar o detener la captura de datos, así como ajustar el valor del PWM manualmente.
 */
void loop() {
  // Verificar si hay comandos disponibles desde el puerto serial.
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    // Comando para iniciar la captura de datos con un valor de PWM específico.
    if (command.startsWith("START")) {
      String valueStr = command.substring(6);
      int value = valueStr.toInt();
      if (value >= 0 && value <= 100) {
        startCapture(value);
      } else {
        Serial.println("Valor fuera de rango. El valor de PWM debe estar entre 0 y 100.");
      }
    }

    // Comando para ajustar el valor de PWM manualmente.
    else if (command.startsWith("PWM")) {
      String valueStr = command.substring(4);
      int value = valueStr.toInt();
      if (value >= 0 && value <= 100) {
        setPWM(value);
      } else {
        Serial.println("Valor fuera de rango. El valor de PWM debe estar entre 0 y 100.");
      }
    }

    // Comando para detener el sistema y desactivar el motor.
    else if (command.equalsIgnoreCase("STOP")) {
      sistemaActivo = false;
      analogWrite(ENA, 0);  ///< Detiene el motor al poner el PWM en 0.
      currentPWM = 0;
      Serial.println("Sistema detenido.");
    }
  }

  // Si el sistema está activo y no se está capturando, se envían datos manualmente.
  if (sistemaActivo && !capturing) {
    static unsigned long lastSendTime = 0;
    if (millis() - lastSendTime >= 500) {
      sendManualData();
      lastSendTime = millis();
    }
  }
}

/**
 * @brief Inicia la captura de datos con un incremento específico en el valor de PWM.
 * 
 * Esta función aplica una secuencia de PWM incremental al motor y calcula la velocidad en RPM
 * a partir de los pulsos del encoder. Los datos se almacenan en los buffers definidos.
 *
 * @param pwmIncrement Valor con el que se incrementará el PWM en cada paso.
 */
void startCapture(int pwmIncrement) {
  capturing = true;  ///< Marca que se está capturando datos.
  sampleIndex = 0;  ///< Reinicia el índice de muestra.
  Serial.print("Iniciando captura con incremento de PWM: ");
  Serial.println(pwmIncrement);

  // Secuencia de PWM (rampa hacia arriba y luego hacia abajo)
  int stepPWM[] = {0, pwmIncrement, 2 * pwmIncrement, 3 * pwmIncrement, 4 * pwmIncrement, 5 * pwmIncrement, 4 * pwmIncrement, 3 * pwmIncrement, 2 * pwmIncrement, pwmIncrement, 0};

  // Aplicación de cada paso de PWM y captura de datos
  for (int i = 0; i < sizeof(stepPWM) / sizeof(stepPWM[0]); i++) {
    int pwm = stepPWM[i];
    analogWrite(ENA, map(pwm, 0, 100, 0, 255));  ///< Configura el valor del PWM.

    unsigned long stepStart = millis();
    while (millis() - stepStart < 2000) {  ///< Duración de cada paso de 2 segundos.
      unsigned long currentTime = millis();
      if (currentTime - lastSampleTime >= 4) {  ///< Intervalo de muestreo (4 ms).
        noInterrupts();
        unsigned int currentCount = pulseCount;
        interrupts();

        unsigned int deltaPulses = currentCount - lastPulseCount;
        lastPulseCount = currentCount;

        // Cálculo de RPM (pulsos por minuto)
        float rpm = (deltaPulses / (float)pulsesPerRev) * (60000.0 / 4);

        // Almacena los datos si hay espacio en los buffers
        if (sampleIndex < maxSamples) {
          timestamp[sampleIndex] = currentTime - startTime;
          pwmBuffer[sampleIndex] = pwm;
          rpmBuffer[sampleIndex] = rpm;
          sampleIndex++;
        }

        lastSampleTime = currentTime;
      }
    }
  }

  // Imprime los datos capturados en formato CSV.
  for (int i = 0; i < sampleIndex; i++) {
    Serial.print(timestamp[i]);
    Serial.print(",");
    Serial.print(pwmBuffer[i]);
    Serial.print(",");
    Serial.println(rpmBuffer[i]);
  }

  Serial.println("Secuencia completada.");
  capturing = false;
}

/**
 * @brief Ajusta el valor del PWM manualmente.
 * 
 * Esta función permite ajustar el valor del PWM en tiempo real, según el comando recibido.
 *
 * @param pwmValue Valor de PWM entre 0 y 100 para establecer.
 */
void setPWM(int pwmValue) {
  currentPWM = pwmValue;
  analogWrite(ENA, map(pwmValue, 0, 100, 0, 255));  ///< Ajusta el valor de PWM.
  Serial.print("PWM ajustado a: ");
  Serial.println(pwmValue);
}

/**
 * @brief Envía los datos manuales de PWM y RPM.
 * 
 * Esta función envía la información de PWM y RPM en intervalos regulares
 * cuando el sistema está activo y no está capturando datos.
 */
void sendManualData() {
  if (currentPWM == 0) return;  ///< No imprime si el PWM es 0.

  noInterrupts();
  unsigned int currentCount = pulseCount;
  interrupts();

  unsigned int deltaPulses = currentCount - lastPulseCount;
  lastPulseCount = currentCount;

  float rpm = (deltaPulses / (float)pulsesPerRev) * (60000.0 / 500);

  Serial.print("PWM: ");
  Serial.print(currentPWM);
  Serial.print(", RPM: ");
  Serial.println(rpm);
}
