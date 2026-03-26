/*
 * Modulino Light - Basic
 * 
 * This example demonstrates how to read color, light intensity, and infrared levels
 * from the Modulino Light sensor.
 * 
 * The sensor can detect:
 * - Color: RGB values and approximate color name (red, green, blue, etc.)
 * - Ambient Light (AL): measured in lux (light intensity)
 * - Infrared (IR): infrared light level
 * 
 * The color data is returned as a 32-bit value with this structure:
 * - Bits 24-31: Red component (0-255)
 * - Bits 16-23: Green component (0-255)
 * - Bits 8-15: Blue component (0-255)
 * - Bits 0-7: unused
 * 
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: CC0-1.0
 */

#include <Arduino_Modulino.h>

// Create a ModulinoLight object
ModulinoLight light;

// Variables to store light sensor readings
int lux;  // Ambient light level
int ir;   // Infrared light level

void setup() {
  Serial.begin(9600);
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to light sensor module
  light.begin();
}

void loop() {
  // Update sensor readings (call this before reading values)
  light.update();
  
  // Get the full color value (32-bit RGBA format)
  ModulinoColor color = light.getColor();
  // Get an approximate color name (e.g., "red", "blue", "green")
  String colorName = light.getColorApproximate();
  Serial.print("Color near to: ");
  Serial.print(colorName);

  // Extract individual RGB components from the 32-bit color value
  // Use bit shifting and masking to get each 8-bit color channel
  int r = (0xFF000000 & color) >> 24;  // Red: bits 24-31
  int g = (0x00FF0000 & color) >> 16;  // Green: bits 16-23
  int b = (0x0000FF00 & color) >> 8;   // Blue: bits 8-15

  // Get ambient light level in lux
  lux = light.getAL();
  // Get infrared light level
  ir = light.getIR();

  // Print all sensor readings in a formatted output
  Serial.print("\tlight data: ");
  Serial.print("\tRed:\t");
  Serial.print(r);
  Serial.print("\tGreen:\t");
  Serial.print(g);
  Serial.print("\tBlue:\t");
  Serial.print(b);
  Serial.print("\tLux:\t");
  Serial.print(lux);
  Serial.print("\tIR\t");
  Serial.println(ir);
}
