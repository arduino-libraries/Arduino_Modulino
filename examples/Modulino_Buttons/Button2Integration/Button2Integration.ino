/*
 * Modulino Buttons - Button2 Integration
 * 
 * This example demonstrates how to integrate the Modulino Buttons module with the
 * Button2 library (https://github.com/LennartHennigs/Button2) to gain advanced
 * button handling features like double-clicks, triple-clicks, and long-press detection.
 * 
 * The Button2 library provides:
 * - Single click detection
 * - Double click detection
 * - Triple click detection  
 * - Long press detection
 * - Click counting
 * 
 * This example uses a virtual button pattern:
 * 1. The Modulino button state is read via button0StateHandler()
 * 2. This state is passed to Button2 which handles all the click pattern detection
 * 3. When a pattern is detected, the handler() function is called with the event type
 * 
 * Hardware required:
 * - Modulino Buttons module
 * 
 * If using Arduino IDE, install Button2 via Library Manager to use this example.
 *
 * This example code is in the public domain. 
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>
#include "Button2.h"

// Create a Button2 object for advanced click detection
Button2 button;
// Create a ModulinoButtons object
ModulinoButtons modulino_buttons;

// This function provides the button state to the Button2 library
// It reads from Modulino and translates to standard Arduino button logic
// Returns LOW when pressed, HIGH when released (standard pull-up logic)
uint8_t button0StateHandler() {
  // Update Modulino button states
  modulino_buttons.update();
  // Return LOW when pressed, HIGH when not pressed
  // This mimics a standard button connected with pull-up resistor
  return modulino_buttons.isPressed(0) ? LOW : HIGH;
}

// This handler is called whenever a click pattern is detected
// The btn parameter contains information about which pattern was detected
void handler(Button2& btn) {
  // Check which type of click pattern occurred
  switch (btn.getType()) {
    case single_click:
      // Single click detected - no prefix printed
      break;
    case double_click:
      Serial.print("double ");
      break;
    case triple_click:
      Serial.print("triple ");
      break;
    case long_click:
      Serial.print("long");
      break;
  }
  Serial.print("click");
  Serial.print(" (");
  // Print total number of clicks detected in this sequence
  Serial.print(btn.getNumberOfClicks());
  Serial.println(")");
}

void setup() {

  Serial.begin(115200);

  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to buttons module
  modulino_buttons.begin();

  // Configure Button2 library settings
  button.setDebounceTime(35);  // Set debounce time to 35ms to filter noise
  button.setButtonStateFunction(button0StateHandler);  // Use our custom state handler
  // Register handler for different click types
  button.setClickHandler(handler);
  button.setDoubleClickHandler(handler);
  button.setTripleClickHandler(handler);
  button.setLongClickHandler(handler);
  // Initialize with a virtual pin (not a real Arduino pin)
  button.begin(BTN_VIRTUAL_PIN);
}

void loop() {
  // Continuously check for button events
  // This must be called frequently for Button2 to detect patterns
  button.loop();
}
