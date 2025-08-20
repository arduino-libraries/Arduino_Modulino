/*
 * Modulino Pixels - Simple Animation
 *
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Create a ModulinoPixels object for the LED array
ModulinoPixels leds;

// Define a custom color for turning off LEDs
ModulinoColor OFF(0, 0, 0);

int brightness = 25;

void setup() {
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to pixels module
  leds.begin();
}

void loop() {
  // Light up LEDs in different colors
  // Available colors: RED, BLUE, GREEN, VIOLET, WHITE
  for (int i = 0; i < 8; i++) {
    if (i == 0 || i == 1) {
      setPixel(i, RED);
    } else if (i == 2 || i == 3) {
      setPixel(i, BLUE);
    } else if(i == 4 || i == 5){
      setPixel(i, GREEN);
    } else if(i == 6 || i == 7){
      setPixel(i, VIOLET);
    } else if (i == 7 || i == 8) {
      setPixel(i, WHITE);
    }

  delay(25);

  }

  // Turn off all LEDs one by one
  for (int i = 0; i < 8; i++) {
    setPixel(i, OFF);
    delay(25);
  }

}

void setPixel(int pixel, ModulinoColor color) {
  leds.set(pixel, color, brightness);
  leds.show();
}
