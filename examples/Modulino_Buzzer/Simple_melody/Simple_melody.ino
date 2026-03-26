/*
 * Modulino Buzzer - Simple melody
 * 
 * This example plays a simple 8-note melody using the Modulino Buzzer.
 * Each note is defined by its frequency in Hertz (Hz).
 * 
 * The melody is the beginning of "Happy Birthday":
 * - C4, G3, G3, A3, G3, rest, B3, C4
 * 
 * Frequency range: The buzzer can play frequencies from approximately 20Hz to 20,000Hz (20kHz)
 * Common musical notes:
 * - C3: 131 Hz, D3: 147 Hz, E3: 165 Hz, F3: 175 Hz, G3: 196 Hz, A3: 220 Hz, B3: 247 Hz
 * - C4: 262 Hz, D4: 294 Hz, E4: 330 Hz, F4: 349 Hz, G4: 392 Hz, A4: 440 Hz, B4: 494 Hz
 * - C5: 523 Hz, D5: 587 Hz, E5: 659 Hz, F5: 698 Hz, G5: 784 Hz, A5: 880 Hz, B5: 988 Hz
 * 
 * A frequency of 0 represents a rest (silence).
 *
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Create a ModulinoBuzzer object
ModulinoBuzzer buzzer;

// Define melody notes in Hz (C4, G3, G3, A3, G3, rest, B3, C4)
// 0 represents a rest (no sound)
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
    // Get current note frequency from the array
    int note = melody[i];
    // Play the note for 250ms (quarter of a second)
    buzzer.tone(note, 250);
    // Wait for the note to finish playing before moving to the next note
    delay(250);

  }
}
