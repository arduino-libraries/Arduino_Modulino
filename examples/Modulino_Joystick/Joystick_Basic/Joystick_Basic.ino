/*
 * Modulino Joystick - Basic
 * 
 * This example demonstrates how to read analog joystick position and button state
 * from the Modulino Joystick module.
 * 
 * The joystick has:
 * - X-axis: horizontal movement (-100 to +100, center is 0)
 * - Y-axis: vertical movement (-100 to +100, center is 0)
 * - Button: can be pressed down (click the joystick)
 * 
 * Coordinate system:
 * - X: negative = left, positive = right
 * - Y: negative = down, positive = up
 * - Center position: X=0, Y=0
 * 
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include "Modulino.h"

// Create a ModulinoJoystick object
ModulinoJoystick joystick;

void setup() {
  Serial.begin(9600);
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to joystick module
  joystick.begin();
}

void loop() {
  // Update joystick readings (call this before reading values)
  joystick.update();

  // Check if the joystick button is currently pressed
  if(joystick.isPressed()) {
    Serial.println("Pressed");
  }

  // Print the X and Y coordinates
  // X: -100 (left) to +100 (right)
  // Y: -100 (down) to +100 (up)
  Serial.print("x,y: ");
  Serial.print(joystick.getX());
  Serial.print(", ");
  Serial.println(joystick.getY());
}
