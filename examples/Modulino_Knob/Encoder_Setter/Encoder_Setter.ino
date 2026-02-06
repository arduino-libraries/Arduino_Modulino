/*
 * Modulino Knob - Encoder Setter
 * 
 * This example demonstrates how to set and constrain the encoder position value
 * and use it to control LED brightness. It also shows how to read button states.
 * 
 * Features demonstrated:
 * - Setting encoder position programmatically with encoder.set()
 * - Constraining encoder values to a specific range (0-100)
 * - Using encoder value to control LED brightness
 * - Reading button states and displaying them on button LEDs
 * 
 * Hardware used:
 * - Modulino Knob (encoder with push button)
 * - Modulino Pixels (LED strip) - uses LEDs 1, 2, 3 for RGB display
 * - Modulino Buttons - for additional button inputs
 * 
 * The encoder position is kept within 0-100 range:
 * - If it goes above 100 or below 0, it's reset to 0
 * - This value is used to set the brightness of 3 LEDs (red, green, blue)
 *
 * This example code is in the public domain. 
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Create objects for the modules
ModulinoKnob encoder;     // Rotary encoder with button
ModulinoPixels leds;      // LED strip
ModulinoButtons buttons;  // Button module

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
  
  // Reset encoder position if out of valid range (0-100)
  // This constrains the encoder to a specific range
  if (value > 100 || value < 0) {
    encoder.set(0);  // Reset to 0 if outside range
  }

  // Get updated encoder value after possible reset
  value = encoder.get();

  // Print current value to Serial Monitor
  Serial.println(value);

  // Set LED brightness based on encoder value (0-100)
  // Using LEDs at positions 1, 2, and 3 for RGB color display
  // Available predefined colors: RED, BLUE, GREEN, VIOLET, WHITE
  leds.set(1, RED, value);    // LED 1: Red with variable brightness
  leds.set(2, GREEN, value);  // LED 2: Green with variable brightness
  leds.set(3, BLUE, value);   // LED 3: Blue with variable brightness
  
  // Update physical LEDs with new settings
  leds.show();
  
  // Update button states (read new button presses)
  buttons.update();
  
  // Set button LEDs to reflect button press states
  // Each button LED turns on when its button is pressed
  // isPressed(0) = Button A, isPressed(1) = Button B, isPressed(2) = Button C
  buttons.setLeds(buttons.isPressed(0), buttons.isPressed(1), buttons.isPressed(2));
}
