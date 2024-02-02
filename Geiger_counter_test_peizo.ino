#include <Arduino.h>

const int geigerPin = 7;   // Pin connected to the Geiger counter
const int ledPin = 13;     // Pin connected to an LED (optional)
const int piezoPin = 1;    // Pin connected to the piezo buzzer

volatile unsigned int pulseCount = 0;
const unsigned long countDuration = 120000; // 2 minutes in milliseconds
const float alpha = 53.032; // uSv to CPM conversion factor (adjust as necessary)

void radiationDetected() {
  static unsigned long lastPulseTime = 0;
  unsigned long currentPulseTime = millis();

  if (currentPulseTime - lastPulseTime > 50) { // Debounce period of 50ms
    pulseCount++;
    lastPulseTime = currentPulseTime;

    tone(piezoPin, 1000, 100); // Beep for 100 milliseconds at 1000 Hz
  }
}

void setup() {
  SerialUSB.begin(115200);
  pinMode(geigerPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(piezoPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(geigerPin), radiationDetected, FALLING);
}

void loop() {
  pulseCount = 0;
  unsigned long startTime = millis();

  while (millis() - startTime < countDuration) {
    digitalWrite(ledPin, (millis() / 100) % 10 < 5); // Blink LED as an indicator
  }

  float cpm = pulseCount;
  float uSv_h = cpm / alpha;

  SerialUSB.print("Counts Per Minute (CPM): ");
  SerialUSB.println(cpm);
  SerialUSB.print("Radiation (uSv/h): ");
  SerialUSB.println(uSv_h);
  SerialUSB.println("----");

  delay(1000); // Short delay before the next measurement
}
