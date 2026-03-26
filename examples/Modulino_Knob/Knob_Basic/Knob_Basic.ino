/*
 * Modulino Knob - Basic
 * 
 * This example demonstrates how to read position, button press, and rotation
 * direction from the Modulino Knob (rotary encoder).
 * 
 * The knob module features:
 * - Rotary encoder: Tracks rotation position (increments/decrements with each turn)
 * - Push button: Can be pressed down (click the knob)
 * - Direction detection: Detects clockwise (1) or counter-clockwise (-1) rotation
 * 
 * Position value:
 * - Starts at 0 by default
 * - Increases when rotated clockwise
 * - Decreases when rotated counter-clockwise
 * - Can be positive or negative (no fixed range)
 * - Can be reset or set to a specific value using knob.set()
 * 
 * Applications:
 * - Volume control
 * - Menu navigation
 * - Parameter adjustment
 * - Value selection
 *
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Create a ModulinoKnob object
ModulinoKnob knob;

void setup() {
  Serial.begin(9600);
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to knob module
  knob.begin();
}

void loop(){
  // Get the current position value of the knob
  // This value increments when turned clockwise, decrements when turned counter-clockwise
  int position = knob.get();
  
  // Check if the knob has been pressed (clicked)
  // Returns true on the first call after a press, then false until next press
  bool click = knob.isPressed();
  
  // Get the rotation direction since last check
  // Returns: 1 for clockwise, -1 for counter-clockwise, 0 for no rotation
  int8_t direction = knob.getDirection();

  // Print the current position value
  Serial.print("Current position is: ");
  Serial.println(position);

  // Print message when knob is clicked
  if(click){
    Serial.println("Clicked!");
  }

  // Print rotation direction
  if (direction == 1) {
    Serial.println("Rotated clockwise");
  } else if (direction == -1) {
    Serial.println("Rotated counter-clockwise");
  }

  // Small delay to reduce serial output spam and improve readability
  delay(10);
} 
