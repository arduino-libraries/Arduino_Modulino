/*
 * Modulino Movement - Basic
 * 
 * This example demonstrates how to read motion data from the Modulino Movement sensor.
 * This sensor contains an IMU (Inertial Measurement Unit) with both an accelerometer
 * and a gyroscope.
 * 
 * Sensor capabilities:
 * - Accelerometer: Measures linear acceleration in 3 axes (X, Y, Z)
 *   Values are in g-force (1g = 9.8 m/s²)
 *   Typical range: -2g to +2g
 * 
 * - Gyroscope: Measures rotational velocity around 3 axes (roll, pitch, yaw)
 *   Values are in degrees per second (°/s)
 *   Roll: Rotation around X-axis (tilting left/right)
 *   Pitch: Rotation around Y-axis (tilting forward/backward)
 *   Yaw: Rotation around Z-axis (spinning clockwise/counterclockwise)
 * 
 * Applications:
 * - Tilt detection
 * - Gesture recognition
 * - Motion tracking
 * - Orientation sensing
 * - Vibration monitoring
 *
 * This example code is in the public domain. 
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Create a ModulinoMovement object
ModulinoMovement movement;


// Variables to store accelerometer readings (g-force)
float x, y, z;
// Variables to store gyroscope readings (degrees/second)
float roll, pitch, yaw;


void setup() {
  Serial.begin(9600);
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to movement sensor module
  movement.begin();
}

void loop() {
  // Read new movement data from the sensor
  // Call this before reading individual values
  movement.update();

  // Get acceleration values (in g-force, where 1g ≈ 9.8 m/s²)
  x = movement.getX();  // X-axis acceleration
  y = movement.getY();  // Y-axis acceleration
  z = movement.getZ();  // Z-axis acceleration (typically ~1.0 when upright due to gravity)
  
  // Get gyroscope values (in degrees per second)
  roll = movement.getRoll();   // Rotation around X-axis (left/right tilt)
  pitch = movement.getPitch(); // Rotation around Y-axis (forward/backward tilt)
  yaw = movement.getYaw();     // Rotation around Z-axis (spinning)

  // Print acceleration values (A:) with 3 decimal places
  Serial.print("A: ");
  Serial.print(x, 3);
  Serial.print(", ");
  Serial.print(y, 3);
  Serial.print(", ");
  Serial.print(z, 3);
  
  // Print divider between acceleration and gyroscope sections
  Serial.print(" | G: ");
  
  // Print gyroscope values (G:) with 1 decimal place
  Serial.print(roll, 1);
  Serial.print(", ");
  Serial.print(pitch, 1);
  Serial.print(", ");
  Serial.println(yaw, 1);
  
  // Delay to make serial output readable (200ms = 5 readings per second)
  delay(200);
}