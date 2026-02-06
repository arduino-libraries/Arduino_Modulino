/*
 * Modulino Buzzer - Basic
 * 
 * This example demonstrates how to use the Modulino Buzzer to play tones.
 * The buzzer plays a 440Hz tone (musical note A4) for 1 second, then stops for 1 second,
 * and repeats.
 * 
 * Frequency range: The buzzer can play frequencies from approximately 20Hz to 20,000Hz (20kHz)
 * Common musical note frequencies:
 * - C4 (Middle C): 262 Hz
 * - A4 (Concert pitch): 440 Hz
 * - C5: 523 Hz
 *
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Create a ModulinoBuzzer object
ModulinoBuzzer buzzer;

// Frequency in Hertz (Hz) - 440Hz is the musical note A4
int frequency = 440;
// Duration in milliseconds (ms) - how long the tone plays
int duration = 1000;

void setup(){
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to buzzer module
  buzzer.begin();
}

void loop(){

  // Play tone at specified frequency (440Hz) for specified duration (1000ms = 1 second)
  buzzer.tone(frequency, duration);
  // Wait for the tone to finish playing
  delay(1000);
  
  // Stop the tone by setting frequency to 0
  buzzer.tone(0, duration);
  // Wait 1 second in silence before repeating
  delay(1000);

}
