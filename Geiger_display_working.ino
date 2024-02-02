/*
Copyright (c) 2023 Apollo Timbers. All rights reserved.

This work is licensed under the terms of the Apache License 2.0.
For a copy, see https://github.com/apache/.github/blob/main/LICENSE.

Geiger counter project:

The sensor and code are for educational purposes and should not be directly relied upon for determinations regarding one’s health or safety.
*/


#include <Arduino.h>
#include <Wire.h>
#include <SparkFun_Alphanumeric_Display.h>

const int geigerPin = 7;
const int ledPin = 13;
const int piezoPin = 1;

volatile unsigned int pulseCount = 0;
const unsigned long countDuration = 120000; // 2 minutes in milliseconds
const float alpha = 53.032; // uSv to CPM conversion factor (adjust as necessary)
volatile bool radiationDetectedFlag = false; // Flag to indicate radiation was detected

HT16K33 display; // Create an instance of the display

void radiationDetected() {
  static unsigned long lastPulseTime = 0;
  unsigned long currentPulseTime = millis();

  if (currentPulseTime - lastPulseTime > 50) { // Debounce period of 50ms
    pulseCount++;
    lastPulseTime = currentPulseTime;

    // Short beep
    digitalWrite(piezoPin, HIGH);
    delayMicroseconds(100); // Very short beep
    digitalWrite(piezoPin, LOW);
  }
}

void setup() {
  SerialUSB.begin(115200);
  while (!SerialUSB) {} // Wait for the SerialUSB port to connect

  pinMode(geigerPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  pinMode(piezoPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(geigerPin), radiationDetected, FALLING);

  Wire.begin(); // Start the I2C bus
  if (display.begin() == false) {
    SerialUSB.println("Display did not acknowledge! Freezing.");
    while (1);
  }
  SerialUSB.println("Display acknowledged.");
}

void scrollText(String text, int delayTime) {
  for (int i = 0; i <= text.length(); i++) {
    display.print(text.substring(i));
    display.updateDisplay();
    delay(delayTime);
  }
}

void loop() {
  pulseCount = 0;
  unsigned long startTime = millis();

  while (millis() - startTime < countDuration) {
    digitalWrite(ledPin, (millis() / 100) % 10 < 5); // Blink LED as an indicator
  }

  // After measurement period
  float cpm = pulseCount;
  float uSv_h = cpm / alpha;

  SerialUSB.print("Counts Per Minute (CPM): ");
  SerialUSB.println(cpm);
  SerialUSB.print("Radiation (uSv/h): ");
  SerialUSB.println(uSv_h);
  SerialUSB.println("----");

   // Create a string with the radiation data
  String message = "Rads uSv/h: " + String(uSv_h, 2); // 2 decimal places

  // Scroll the message
  scrollText(message, 300); // Scroll speed - 300 ms per character

  delay(1000); // Short delay before the next measurement
}
