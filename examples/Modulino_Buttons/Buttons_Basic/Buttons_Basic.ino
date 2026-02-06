/*
 * Modulino Buttons - Basic
 * 
 * This example demonstrates how to read button presses from the Modulino Buttons module
 * and control the LEDs above each button.
 * 
 * The Modulino Buttons module features:
 * - Three push buttons labeled A, B, and C
 * - Three LEDs above the buttons (one above each button)
 * - Buttons can be checked by index (0, 1, 2) or by letter ('A', 'B', 'C')
 * 
 * In this example:
 * - Each button press toggles a boolean variable
 * - The corresponding LED reflects the state of that variable
 * - LED on = true, LED off = false
 * 
 * Usage:
 * - Press button A to toggle LED A
 * - Press button B to toggle LED B
 * - Press button C to toggle LED C
 * 
 * Applications:
 * - User input interface
 * - Menu navigation
 * - On/off controls
 * - State indicators
 *
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Create a ModulinoButtons object
ModulinoButtons buttons;

// Variables to store the LED states for each button
// true = LED on, false = LED off
bool button_a = true;
bool button_b = true;
bool button_c = true;

void setup() {
  Serial.begin(9600);
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to buttons module
  buttons.begin();
  // Turn on all three LEDs above buttons A, B, and C
  // Parameters: setLeds(LED_A, LED_B, LED_C)
  buttons.setLeds(true, true, true);
}

void loop() {
  // Check for new button events
  // Returns true when any button state changes (pressed or released)
  if (buttons.update()) {
    // You can use either index (0=A, 1=B, 2=C) or letter ('A', 'B', 'C') to check buttons
    // Below we use the letter-based method for better readability
    // Note: 'A' and 'C' use single quotes (char), "B" uses double quotes (string) - both work

    if (buttons.isPressed('A')) {
      Serial.println("Button A pressed!");
      // Toggle the state (true becomes false, false becomes true)
      button_a = !button_a;
    } else if (buttons.isPressed("B")) {
      Serial.println("Button B pressed!");
      button_b = !button_b;
    } else if (buttons.isPressed('C')) {
      Serial.println("Button C pressed!");
      button_c = !button_c;
    }

    // Update the LEDs above buttons to reflect the current variable states
    buttons.setLeds(button_a, button_b, button_c);
  }
}
