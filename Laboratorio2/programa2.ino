const int ENA = 0;
const int IN1 = 1;
const int IN2 = 2;
const int ENCODER_PIN = 10;

volatile unsigned int pulseCount = 0;
unsigned int lastPulseCount = 0;
unsigned int pulsesPerRev = 20;

const int stepPWM[] = {0, 20, 40, 60, 80, 100, 80, 60, 40, 20, 0}; 
const int stepDuration = 2000; // 2 segundos
const int sampleInterval = 4;  // 4 ms = 250 Hz

const int maxSamples = 5000;  // Buffer suficientemente grande
unsigned long timestamp[maxSamples];
int pwmBuffer[maxSamples];
float rpmBuffer[maxSamples];
int sampleIndex = 0;

unsigned long lastSampleTime = 0;
unsigned long startTime = 0;

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
  for (int i = 0; i < sizeof(stepPWM) / sizeof(stepPWM[0]); i++) {
    int pwm = stepPWM[i];
    analogWrite(ENA, map(pwm, 0, 100, 0, 255));

    unsigned long stepStart = millis();
    while (millis() - stepStart < stepDuration) {
      unsigned long currentTime = millis();
      if (currentTime - lastSampleTime >= sampleInterval) {
        // Medir diferencia de pulsos
        noInterrupts();
        unsigned int currentCount = pulseCount;
        interrupts();

        unsigned int deltaPulses = currentCount - lastPulseCount;
        lastPulseCount = currentCount;

        // Calcular RPM basado en delta de pulsos
        float rpm = (deltaPulses / (float)pulsesPerRev) * (60000.0 / sampleInterval);

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
  while (true); // Detener
}
