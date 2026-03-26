/*
 * Modulino Light - Color Detection
 * 
 * This example creates a color mixer using three knobs to control RGB values
 * and displays the mixed color on a Modulino Pixels LED strip.
 * 
 * Hardware Setup:
 * - 3x Modulino Knob modules (addresses must be changed to 0x08, 0x09, 0x0A)
 * - 1x Modulino Pixels (LED strip)
 * - 1x Modulino Light sensor
 * 
 * IMPORTANT: Before running this sketch, you must change the I2C addresses of the
 * three encoder modules using the AddressChanger utility example. By default, all
 * encoders use the same address and will conflict. Set them to:
 * - Green knob: 0x08 (default)
 * - Red knob: 0x09
 * - Blue knob: 0x0A
 * 
 * How it works:
 * - Each knob controls one color channel (Red, Green, or Blue) from 0-255
 * - Turn a knob to adjust its color value
 * - Click a knob to set that color to maximum (255)
 * - Click multiple knobs simultaneously for color combinations:
 *   - All three: Sets all to 0 (black/off)
 *   - Red + Green: Yellow (255, 255, 0)
 *   - Red + Blue: Magenta (255, 0, 255)
 *   - Green + Blue: Cyan (0, 255, 255)
 * - The resulting color is shown on all 8 LEDs
 * - The light sensor detects and displays the approximate color name
 * 
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: CC0-1.0
 */

#include <Arduino_Modulino.h>

// Create a ModulinoLight object for color sensing
ModulinoLight light;

// Create three ModulinoKnob objects with different I2C addresses
// These addresses must be set using the AddressChanger utility first
ModulinoKnob knob_green;        // Default address 0x08
ModulinoKnob knob_red(0x09);    // Custom address 0x09
ModulinoKnob knob_blue(0x0A);   // Custom address 0x0A

// Create a ModulinoPixels object for the LED strip
ModulinoPixels leds;


// Variables to store RGB color values (0-255 for each channel)
int brightness = 100;  // LED brightness level (0-100)
int red = 0;           // Red color component
int green = 0;         // Green color component
int blue = 0;          // Blue color component


void setup() {
  Serial.begin(9600);
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to all three knob modules
  knob_green.begin();
  knob_red.begin();
  knob_blue.begin();

  // Set all knobs' initial positions to 0
  knob_red.set(0);
  knob_green.set(0);
  knob_blue.set(0);
  
  // Detect and connect to light sensor and LED modules
  light.begin();
  leds.begin();
}

// Timer variable for periodic serial output (every 1 second)
unsigned long start = millis();

void loop() {
  // Handle knob button presses for special color combinations
  knobPressed();
  // Enforce valid RGB ranges (0-255)
  readColors();
  // Update the LED strip with the current color
  updatesColors();
}

// Validates and constrains RGB values to the valid range (0-255)
// If a value exceeds limits, it's clamped and the knob is reset to match
void readColors() {
  // Constrain red value to 0-255 range
  if (red > 255) {
    red = 255;
    knob_red.set(red);
  } else if (red < 0) {
    red = 0;
    knob_red.set(red);
  }

  // Constrain green value to 0-255 range
  if (green > 255) {
    green = 255;
    knob_green.set(green);
  } else if (green < 0) {
    green = 0;
    knob_green.set(green);
  }

  // Constrain blue value to 0-255 range
  if (blue > 255) {
    blue = 255;
    knob_blue.set(blue);
  } else if (blue < 0) {
    blue = 0;
    knob_blue.set(blue);
  }
}

// Handles knob button presses to set special color values and combinations
void knobPressed() {
  // Read current knob positions (0-255)
  red = knob_red.get();
  green = knob_green.get();
  blue = knob_blue.get();
  
  // Check if each knob button is pressed (clicked)
  bool red_click = knob_red.isPressed();
  bool green_click = knob_green.isPressed();
  bool blue_click = knob_blue.isPressed();

  // Single knob press: Set that color to maximum (255)
  if (red_click) {
    red = 255;
    knob_red.set(red);
  }

  if (green_click) {
    green = 255;
    knob_green.set(green);
  }

  if (blue_click) {
    blue = 255;
    knob_blue.set(blue);
  }

  // Multi-knob press combinations for preset colors
  if (green_click && blue_click && red_click ) {
    // All three pressed: Reset to black (all off)
    red = 0;
    knob_red.set(red);
    green = 0;
    knob_green.set(green);
    blue = 0;
    knob_blue.set(blue);
  } else if (red_click && green_click) {
    // Red + Green pressed: Yellow
    red = 255;
    knob_red.set(red);
    green = 255;
    knob_green.set(green);
    blue = 0;
    knob_blue.set(blue);
  } else if (red_click && blue_click) {
    // Red + Blue pressed: Magenta
    red = 255;
    knob_red.set(red);
    green = 0;
    knob_green.set(green);
    blue = 255;
    knob_blue.set(blue);
  } else if (green_click && blue_click) {
    // Green + Blue pressed: Cyan
    red = 0;
    knob_red.set(red);
    green = 255;
    knob_green.set(green);
    blue = 255;
    knob_blue.set(blue);
  }
}

// Updates the LED strip with the current RGB color and prints debug info to serial
void updatesColors() {
  // Create a custom color from the current RGB values
  ModulinoColor COLOR(red, green, blue);
  
  // Set all 8 LEDs to the selected color
  for (int l = 0; l < 8; l++) {
    leds.set(l, COLOR, brightness);
    leds.show();
  }
  
  // Update the light sensor reading
  light.update();

  // Print color values to serial monitor every 1 second
  if (millis() - start > 1000) {
    char buffer [50];
    int n, a = 3;
    
    // Format and print red value with leading zeros
    n = sprintf (buffer, "%03d", red);

    Serial.print("Red:\t");
    Serial.print(buffer);
    
    // Format and print green value with leading zeros
    n = sprintf (buffer, "%03d", green);
    Serial.print("\tGreen:\t");
    Serial.print(buffer);
    
    // Format and print blue value with leading zeros
    n = sprintf (buffer, "%03d", blue);
    Serial.print("\tBlue:\t");
    Serial.print(buffer);

    // Get detected color from the light sensor
    ModulinoColor color = light.getColor();
    String colorName = light.getColorApproximate();
    Serial.print("\tColor near to:\t");
    Serial.print(colorName);
    Serial.println();

    // Reset the timer for next update
    start = millis();
  }
}
