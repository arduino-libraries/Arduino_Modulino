/*
 * Modulino Pixels - Simple Animation
 * 
 * This example creates a simple animation on the Modulino Pixels LED strip.
 * The strip has 8 individually addressable RGB LEDs (indexed 0-7).
 * 
 * Animation sequence:
 * 1. Turn on LEDs one by one in different colors (pairs of 2)
 *    - LEDs 0-1: Red
 *    - LEDs 2-3: Blue
 *    - LEDs 4-5: Green
 *    - LEDs 6-7: Violet
 * 2. Turn off all LEDs one by one
 * 3. Repeat
 * 
 * Available predefined colors: RED, BLUE, GREEN, VIOLET, WHITE
 * You can also create custom colors using ModulinoColor(red, green, blue)
 * where each value is 0-255.
 *
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Create a ModulinoPixels object for the LED array
ModulinoPixels leds;

// Define a custom color for turning off LEDs (all RGB values at 0)
ModulinoColor OFF(0, 0, 0);

// Brightness level (0-100) - lower values save power and reduce intensity
int brightness = 25;

void setup() {
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to pixels module
  leds.begin();
}

void loop() {
  // Light up LEDs one by one in different colors
  // The Modulino Pixels has 8 LEDs indexed from 0 to 7
  for (int i = 0; i < 8; i++) {
    // LEDs 0-1: Red
    if (i == 0 || i == 1) {
      setPixel(i, RED);
    } 
    // LEDs 2-3: Blue
    else if (i == 2 || i == 3) {
      setPixel(i, BLUE);
    } 
    // LEDs 4-5: Green
    else if(i == 4 || i == 5){
      setPixel(i, GREEN);
    } 
    // LEDs 6-7: Violet
    else if(i == 6 || i == 7){
      setPixel(i, VIOLET);
    }

    // Small delay between turning on each LED for animation effect
    delay(25);
  }

  // Turn off all LEDs one by one to create a fade-out effect
  for (int i = 0; i < 8; i++) {
    setPixel(i, OFF);
    delay(25);
  }

}

// Helper function to set a pixel color and update the display
// Parameters:
//   pixel: LED index (0-7)
//   color: ModulinoColor object with RGB values
void setPixel(int pixel, ModulinoColor color) {
  // Set the color for the specified LED
  leds.set(pixel, color, brightness);
  // Update the physical LEDs (must call show() to apply changes)
  leds.show();
}
