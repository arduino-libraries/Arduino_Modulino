/*
 * Modulino Buzzer - Basic
 *
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Create a ModulinoBuzzer object
ModulinoBuzzer buzzer;

int frequency = 440;
int duration = 1000;

void setup(){
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to buzzer module
  buzzer.begin();
}

void loop(){

  // Play tone at specified frequency and duration
  buzzer.tone(frequency, duration);
  delay(1000);
  // Stop the tone (0 frequency)
  buzzer.tone(0, duration);
  delay(1000);

}
