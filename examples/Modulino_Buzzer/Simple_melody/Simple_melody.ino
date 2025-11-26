/*
 * Modulino Buzzer - Simple melody
 *
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Create a ModulinoBuzzer object
ModulinoBuzzer buzzer;

// Define melody notes in Hz (C4, G3, G3, A3, G3, rest, B3, C4)
int melody[] = { 262, 196, 196, 220, 196, 0, 247, 262 };

void setup() {
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to buzzer module
  buzzer.begin();
}

void loop() {
  // Play each note in the melody
  for (int i = 0; i < 8; i++) {
    // Get current note frequency
    int note = melody[i];
    // Play the note for 250ms
    buzzer.tone(note, 250);
    delay(250);

  }
}
