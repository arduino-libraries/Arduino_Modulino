/*
 * Modulino Vibro - Basic
 * 
 * This example demonstrates how to control the Modulino vibration motor.
 * 
 * The vibro motor supports different intensity levels:
 * - GENTLE: Soft vibration (value: typically around 30)
 * - MAXIMUM: Strongest vibration (value: typically 255)
 * - Custom values: You can use any value between GENTLE and MAXIMUM
 * 
 * The on() function has two modes:
 * - Blocking mode (default): Waits until vibration completes before continuing
 * - Non-blocking mode: Returns immediately, vibration continues in background
 * 
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include "Modulino.h"

// Create a ModulinoVibro object
ModulinoVibro vibro;

void setup() {
  Serial.begin(9600);
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to vibration motor module
  vibro.begin();
}

void loop() {
  // Cycle through vibration intensities from GENTLE to MAXIMUM
  // Increment by 5 for each step, with each intensity lasting 1 second
  for (int intensity = GENTLE; intensity <= MAXIMUM; intensity += 5) {
    Serial.println(intensity);
    // Turn on vibration for 1000ms (1 second)
    // Parameters: duration (ms), blocking mode (true), intensity level
    vibro.on(1000, true, intensity);
    // Turn off vibration
    vibro.off();
    // Short pause between intensity levels
    delay(100);
  }

  // Demonstrate blocking mode with simple vibration pattern
  // Two 1-second vibrations separated by a 5-second pause
  delay(1000);
  // First vibration - blocking call waits until complete
  vibro.on(1000);
  delay(5000);
  // Second vibration - blocking call waits until complete
  vibro.on(1000);
  delay(1000);
}
