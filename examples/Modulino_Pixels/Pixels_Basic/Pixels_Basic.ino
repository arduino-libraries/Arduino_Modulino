/*
 * Modulino Pixels - Basic
 * 
 * This example demonstrates how to control the Modulino Pixels LED strip.
 * The Modulino Pixels module has 8 individually addressable RGB LEDs (indexed 0-7).
 * 
 * Each LED can be set to different colors and brightness levels independently.
 * 
 * Available predefined colors:
 * - RED, BLUE, GREEN, VIOLET, WHITE
 * - You can also create custom colors using ModulinoColor(red, green, blue)
 *   where each RGB value ranges from 0-255
 * 
 * Brightness ranges from 0 (off) to 100 (maximum brightness)
 * Lower brightness values save power and reduce LED intensity.
 *
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Create a ModulinoPixels object
ModulinoPixels leds;

// Set brightness level (0-100)
// 25 provides good visibility while saving power
int brightness = 25;

void setup(){
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to pixels module
  leds.begin();
}

void loop(){
  // Set all 8 LEDs to blue color
  // The module has 8 LEDs indexed from 0 to 7
  for (int i = 0; i < 8; i++) {
    // Set each LED: set(index, color, brightness)
    // index: LED position (0-7)
    // color: Predefined color constant or custom ModulinoColor
    // brightness: Intensity level (0-100)
    leds.set(i, BLUE, brightness);
    // Update the physical LEDs with new settings
    // Must call show() to apply the changes to the hardware
    leds.show();
  }
}
