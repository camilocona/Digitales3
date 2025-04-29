const int ENA = 0;
const int IN1 = 1;
const int IN2 = 2;
const int ENCODER_PIN = 10;

volatile unsigned int pulseCount = 0;
unsigned int lastPulseCount = 0;
unsigned int pulsesPerRev = 20;

const int maxSamples = 5000;  // Buffer suficientemente grande
unsigned long timestamp[maxSamples];
int pwmBuffer[maxSamples];
float rpmBuffer[maxSamples];
int sampleIndex = 0;

unsigned long lastSampleTime = 0;
unsigned long startTime = 0;
bool capturing = false;  // Indica si el sistema está capturando datos

void countPulse() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  for (int i = 0; i < 10; i++) Serial.println();

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENCODER_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), countPulse, RISING);

  analogWriteFreq(1000);
  analogWriteRange(255);

  digitalWrite(IN1, HIGH); // Dirección fija
  digitalWrite(IN2, LOW);

  startTime = millis();

  Serial.println("timestamp_ms,pwm_percent,rpm"); // Encabezado CSV
}

void loop() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    // Comando START <valor>
    if (command.startsWith("START")) {
      String valueStr = command.substring(6);  // Obtener valor después de "START "
      int value = valueStr.toInt();
      if (value >= 0 && value <= 100) {
        startCapture(value);
      } else {
        Serial.println("Valor fuera de rango. El valor de PWM debe estar entre 0 y 100.");
      }
    }

    // Comando PWM <valor>
    else if (command.startsWith("PWM")) {
      String valueStr = command.substring(4);  // Obtener valor después de "PWM "
      int value = valueStr.toInt();
      if (value >= 0 && value <= 100) {
        setPWM(value);
      } else {
        Serial.println("Valor fuera de rango. El valor de PWM debe estar entre 0 y 100.");
      }
    }
  }

  if (capturing) {
    // Continuar la captura de la curva experimental
    captureCurve();
  } else {
    // Modo manual: enviar PWM y RPM cada 0.5 segundos (2 Hz)
    static unsigned long lastSendTime = 0;
    if (millis() - lastSendTime >= 500) {
      sendManualData();
      lastSendTime = millis();
    }
  }
}

void startCapture(int pwmIncrement) {
  // Iniciar captura de la curva experimental
  capturing = true;
  sampleIndex = 0;
  Serial.print("Iniciando captura con incremento de PWM: ");
  Serial.println(pwmIncrement);

  // Generar los valores de PWM en pasos
  int stepPWM[] = {0, pwmIncrement, 2 * pwmIncrement, 3 * pwmIncrement, 4 * pwmIncrement, 5 * pwmIncrement, 4 * pwmIncrement, 3 * pwmIncrement, 2 * pwmIncrement, pwmIncrement, 0};

  for (int i = 0; i < sizeof(stepPWM) / sizeof(stepPWM[0]); i++) {
    int pwm = stepPWM[i];
    analogWrite(ENA, map(pwm, 0, 100, 0, 255));

    unsigned long stepStart = millis();
    while (millis() - stepStart < 2000) { // 2 segundos por paso
      unsigned long currentTime = millis();
      if (currentTime - lastSampleTime >= 4) {  // 250 Hz = 4 ms
        // Medir diferencia de pulsos
        noInterrupts();
        unsigned int currentCount = pulseCount;
        interrupts();

        unsigned int deltaPulses = currentCount - lastPulseCount;
        lastPulseCount = currentCount;

        // Calcular RPM basado en delta de pulsos
        float rpm = (deltaPulses / (float)pulsesPerRev) * (60000.0 / 4);  // RPM por minuto

        // Guardar en buffer
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

  // Terminado el ciclo, imprimir todo
  for (int i = 0; i < sampleIndex; i++) {
    Serial.print(timestamp[i]);
    Serial.print(",");
    Serial.print(pwmBuffer[i]);
    Serial.print(",");
    Serial.println(rpmBuffer[i]);
  }

  Serial.println("Secuencia completada.");
  capturing = false;  // Finaliza la captura
}

void captureCurve() {
  // Este es el ciclo donde capturamos los datos de la curva.
  // La funcionalidad ya estaba definida dentro de startCapture(), así que esta función ahora es redundante.
  // No es necesario tener esta función a menos que quieras mover más código de "startCapture" aquí.
}

void setPWM(int pwmValue) {
  // Ajustar el PWM manualmente
  analogWrite(ENA, map(pwmValue, 0, 100, 0, 255));
  Serial.print("PWM ajustado a: ");
  Serial.println(pwmValue);
}

void sendManualData() {
  // Enviar datos de PWM y RPM cada 0.5 segundos
  noInterrupts();
  unsigned int currentCount = pulseCount;
  interrupts();

  unsigned int deltaPulses = currentCount - lastPulseCount;
  lastPulseCount = currentCount;

  // Calcular RPM basado en delta de pulsos
  float rpm = (deltaPulses / (float)pulsesPerRev) * (60000.0 / 500);  // RPM por minuto a 2Hz

  // Enviar datos por la interfaz serial
  Serial.print("PWM: ");
  Serial.print(analogRead(ENA) * 100 / 255);  // Convertir el valor de 0-255 a porcentaje
  Serial.print(", RPM: ");
  Serial.println(rpm);
}

