/**
 * @brief Programa para capturar la curva de reacción de un motor DC
 * @author Tú
 */

const int ENA = 0;
const int IN1 = 1;
const int IN2 = 2;
const int ENCODER_PIN = 10;

volatile unsigned int pulseCount = 0;
unsigned int pulsesPerRev = 20;
float wheelDiameterMM = 25.0;

const int stepPWM[] = {0, 20, 40, 60, 80, 100, 80, 60, 40, 20, 0}; // Escalones de subida y bajada
const int stepDuration = 2000; // 2 segundos entre pasos
const int sampleInterval = 4;  // 4 ms = 250 Hz

struct DataPoint {
  unsigned long timestamp;
  int pwm;
  float rpm;
};

const int maxSamples = 3000;
DataPoint buffer[maxSamples];
int sampleIndex = 0;

unsigned long lastSampleTime = 0;
unsigned long startTime;

void countPulse() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENCODER_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), countPulse, RISING);

  analogWriteFreq(1000);
  analogWriteRange(255);

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  startTime = millis();
}

void loop() {
  for (int i = 0; i < sizeof(stepPWM)/sizeof(stepPWM[0]); i++) {
    int pwm = stepPWM[i];
    analogWrite(ENA, map(pwm, 0, 100, 0, 255));

    unsigned long stepStart = millis();
    while (millis() - stepStart < stepDuration) {
      if (millis() - lastSampleTime >= sampleInterval && sampleIndex < maxSamples) {
        noInterrupts();
        unsigned int count = pulseCount;
        pulseCount = 0;
        interrupts();

        float rpm = (count / (float)pulsesPerRev) * 60.0;
        buffer[sampleIndex++] = {millis() - startTime, pwm, rpm};

        lastSampleTime = millis();
      }
    }
  }

  // Enviar datos CSV al finalizar
  Serial.println("timestamp_ms,pwm_percent,rpm");
  for (int i = 0; i < sampleIndex; i++) {
    Serial.print(buffer[i].timestamp);
    Serial.print(",");
    Serial.print(buffer[i].pwm);
    Serial.print(",");
    Serial.println(buffer[i].rpm);
  }

  while (true); // Detener
}
