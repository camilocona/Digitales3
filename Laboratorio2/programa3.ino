const int ENA = 0;
const int IN1 = 1;
const int IN2 = 2;
const int ENCODER_PIN = 10;

volatile unsigned int pulseCount = 0;
unsigned int lastPulseCount = 0;
unsigned int pulsesPerRev = 20;

const int maxSamples = 5000;
unsigned long timestamp[maxSamples];
int pwmBuffer[maxSamples];
float rpmBuffer[maxSamples];
int sampleIndex = 0;

unsigned long lastSampleTime = 0;
unsigned long startTime = 0;
bool capturing = false;
bool sistemaActivo = true;

int currentPWM = 0;  // PWM aplicado manualmente

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

    if (command.startsWith("START")) {
      String valueStr = command.substring(6);
      int value = valueStr.toInt();
      if (value >= 0 && value <= 100) {
        startCapture(value);
      } else {
        Serial.println("Valor fuera de rango. El valor de PWM debe estar entre 0 y 100.");
      }
    }

    else if (command.startsWith("PWM")) {
      String valueStr = command.substring(4);
      int value = valueStr.toInt();
      if (value >= 0 && value <= 100) {
        setPWM(value);
      } else {
        Serial.println("Valor fuera de rango. El valor de PWM debe estar entre 0 y 100.");
      }
    }

    else if (command.equalsIgnoreCase("STOP")) {
      sistemaActivo = false;
      analogWrite(ENA, 0);
      currentPWM = 0;
      Serial.println("Sistema detenido.");
    }
  }

  if (sistemaActivo) {
    if (capturing) {
      captureCurve();
    } else {
      static unsigned long lastSendTime = 0;
      if (millis() - lastSendTime >= 500) {
        sendManualData();
        lastSendTime = millis();
      }
    }
  }
}

void startCapture(int pwmIncrement) {
  capturing = true;
  sampleIndex = 0;
  Serial.print("Iniciando captura con incremento de PWM: ");
  Serial.println(pwmIncrement);

  int stepPWM[] = {0, pwmIncrement, 2 * pwmIncrement, 3 * pwmIncrement, 4 * pwmIncrement, 5 * pwmIncrement, 4 * pwmIncrement, 3 * pwmIncrement, 2 * pwmIncrement, pwmIncrement, 0};

  for (int i = 0; i < sizeof(stepPWM) / sizeof(stepPWM[0]); i++) {
    int pwm = stepPWM[i];
    analogWrite(ENA, map(pwm, 0, 100, 0, 255));

    unsigned long stepStart = millis();
    while (millis() - stepStart < 2000) {
      unsigned long currentTime = millis();
      if (currentTime - lastSampleTime >= 4) {
        noInterrupts();
        unsigned int currentCount = pulseCount;
        interrupts();

        unsigned int deltaPulses = currentCount - lastPulseCount;
        lastPulseCount = currentCount;

        float rpm = (deltaPulses / (float)pulsesPerRev) * (60000.0 / 4);

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

void captureCurve() {
  // Vacía por ahora (ya implementada dentro de startCapture)
}

void setPWM(int pwmValue) {
  currentPWM = pwmValue;
  analogWrite(ENA, map(pwmValue, 0, 100, 0, 255));
  Serial.print("PWM ajustado a: ");
  Serial.println(pwmValue);
}

void sendManualData() {
  if (currentPWM == 0) return;  // No imprimir si PWM es 0

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

