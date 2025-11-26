/*
 * Modulino Knob - Encoder Setter
 *
 * This example code is in the public domain. 
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Create objects for the modules
ModulinoKnob encoder;
ModulinoPixels leds;
ModulinoButtons buttons;

void setup() {
  Serial.begin(115200);
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to modules
  encoder.begin();
  leds.begin();
  buttons.begin();
}

void loop() {
  // Get current encoder position value
  int value = encoder.get();
  // Reset encoder position if out of range (0-100)
  if (value > 100 || value < 0) {
    encoder.set(0);
  }

  // Get updated encoder value after possible reset
  value = encoder.get();

  Serial.println(value);

  // Set LED brightness based on encoder value (0-100)
  // Available colors: RED, BLUE, GREEN, VIOLET, WHITE
  leds.set(1, RED, value);
  leds.set(2, GREEN, value);
  leds.set(3, BLUE, value);
  // Update LEDs with new settings
  leds.show();
  // Check button states
  buttons.update();
  // Set button LEDs to match button press states
  buttons.setLeds(buttons.isPressed(0), buttons.isPressed(1), buttons.isPressed(2));
}
