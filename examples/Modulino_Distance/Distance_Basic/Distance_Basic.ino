/*
 * Modulino Distance - Basic
 *
 * This example code is in the public domain. 
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Create a ModulinoDistance object
ModulinoDistance distance;

void setup() {
  Serial.begin(9600);
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to distance sensor module
  distance.begin();
}

void loop() {
  // Check if new distance measurement is available
  if (distance.available()) {
    // Get the latest distance measurement in millimeters
    int measure = distance.get();
    Serial.println(measure);
  }
  delay(10);
}