/*
 * Modulino Distance - Basic
 * 
 * This example demonstrates how to read distance measurements from the
 * Modulino Distance sensor.
 * 
 * The distance sensor uses Time-of-Flight (ToF) technology to measure
 * distances accurately by measuring the time it takes for light to
 * reflect back from an object.
 * 
 * Sensor specifications:
 * - Measurement range: Typically 0mm to 4000mm (4 meters)
 * - Accuracy: ±3% of distance
 * - Measurement unit: Millimeters (mm)
 * 
 * The available() method checks if a new measurement is ready before
 * reading to ensure you get fresh data.
 * 
 * Applications:
 * - Obstacle detection
 * - Proximity sensing
 * - Object tracking
 * - Height measurement
 * - Parking assistance
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
  // This ensures we only read when fresh data is ready
  if (distance.available()) {
    // Get the latest distance measurement in millimeters (mm)
    // Values typically range from 0mm to 4000mm
    int measure = distance.get();
    Serial.println(measure);
  }
  // Small delay to control measurement frequency
  delay(10);
}