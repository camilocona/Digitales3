// Pines del motor A (puente H)
const int ENA = 0;
const int IN1 = 1;
const int IN2 = 2;

// Encoder reubicado a GPIO10
const int ENCODER_PIN = 10;

// Parámetros del encoder y rueda
volatile unsigned int pulseCount = 0;
unsigned long lastRPMCheck = 0;
unsigned int pulsesPerRevolution = 20;  // Muescas del encoder
const float wheelDiameterMM = 25.0;     // Diámetro de la rueda [mm]

// Control y cálculo
int dutyCycle = -1;
bool directionSet = false;
bool ready = false;
float rpm = 0;
float velocityKmh = 0;

// Interrupción para contar pulsos
void countPulse() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  pinMode(ENCODER_PIN, INPUT_PULLUP);
  attachInterrupt(ENCODER_PIN, countPulse, RISING);

  analogWriteFreq(1000);
  analogWriteRange(255);

  Serial.println("Ingrese el duty cycle (0–100):");
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (dutyCycle == -1) {
      int val = input.toInt();
      if (val >= 0 && val <= 100) {
        dutyCycle = val;
        analogWrite(ENA, map(dutyCycle, 0, 100, 0, 255));
        Serial.println("Duty recibido.");
        Serial.println("¿Dirección? 'f' (adelante) o 'r' (reversa):");
      } else {
        Serial.println("Duty inválido. Intente de nuevo (0–100):");
      }
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
    }
  }

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
