/**
 * @brief Programa para capturar en vivo la curva de reacción de un motor DC
 */

const int ENA = 0;
const int IN1 = 1;
const int IN2 = 2;
const int ENCODER_PIN = 10;

volatile unsigned int pulseCount = 0;
unsigned int pulsesPerRev = 20;

const int stepPWM[] = {0, 20, 40, 60, 80, 100, 80, 60, 40, 20, 0}; 
const int stepDuration = 2000; // 2 segundos por escalón
const int sampleInterval = 4;  // 4 ms = 250 Hz

unsigned long lastSampleTime = 0;
unsigned long startTime = 0;

void countPulse() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  delay(2000); // Tiempo para abrir Serial Monitor
  for (int i = 0; i < 10; i++) {
  Serial.println(); // Mandar líneas en blanco
}


  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENCODER_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), countPulse, RISING);

  analogWriteFreq(1000);
  analogWriteRange(255);

  digitalWrite(IN1, HIGH); // Motor en dirección fija
  digitalWrite(IN2, LOW);

  startTime = millis();

  Serial.println("timestamp_ms,pwm_percent,rpm"); // Encabezado CSV
}

void loop() {
  for (int i = 0; i < sizeof(stepPWM) / sizeof(stepPWM[0]); i++) {
    int pwm = stepPWM[i];
    analogWrite(ENA, map(pwm, 0, 100, 0, 255));

    /*Serial.print("Cambiando PWM a: ");
    Serial.print(pwm);
    Serial.println("%");*/

    unsigned long stepStart = millis();
    while (millis() - stepStart < stepDuration) {
      unsigned long currentTime = millis();
      if (currentTime - lastSampleTime >= sampleInterval) {
        noInterrupts();
        unsigned int count = pulseCount;
        pulseCount = 0;
        interrupts();

        float rpm = (count / (float)pulsesPerRev) * 60.0;

        Serial.print(currentTime - startTime);
        Serial.print(",");
        Serial.print(pwm);
        Serial.print(",");
        Serial.println(rpm);

        lastSampleTime = currentTime;
      }
    }
  }

  Serial.println("Secuencia completada.");
  while (true); // Terminar
}

