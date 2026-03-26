/*
 * Modulino Thermo - Temperature Humidity Matrix
 * 
 * This example reads temperature and humidity from the Modulino Thermo sensor
 * and displays the values on the Arduino UNO R4 WiFi's built-in LED matrix.
 * 
 * Hardware required:
 * - Arduino UNO R4 WiFi (has built-in 12x8 LED matrix)
 * - Modulino Thermo (temperature and humidity sensor)
 * 
 * Display format:
 * - Temperature is shown in Celsius with one decimal place (e.g., "23.5°C")
 * - Humidity is shown as a percentage with no decimal places (e.g., "45%")
 * - Values scroll across the LED matrix from right to left
 * 
 * The sensor measures:
 * - Temperature: Typically ranges from -40°C to +85°C
 * - Humidity: Relative humidity from 0% to 100%
 * 
 * Note: This example is specifically designed for Arduino UNO R4 WiFi.
 * For other boards, remove the LED matrix code and use Serial.println() instead.
 *
 * This example code is in the public domain. 
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

// Create a ModulinoThermo object
ModulinoThermo thermo;
// Create an object to control the LED matrix on UNO R4 WiFi
ArduinoLEDMatrix matrix;

// Variables to store sensor readings
float temperature = -273.15;  // Initialize to absolute zero (invalid reading)
float humidity = 0.0;

void setup() {
  Serial.begin(9600);

  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to temperature/humidity sensor module
  thermo.begin();

  // Initialize the LED matrix on UNO R4 WiFi
  matrix.begin();
  delay(100);
}

void loop() {
  // Acquire temperature in Celsius and humidity percentage
  temperature = thermo.getTemperature();
  humidity = thermo.getHumidity();

  // Convert the temperature float to a string with 1 decimal point shown
  // and add the °C symbol at the end
  String temperature_text = String(temperature, 1) + "°C";

  // Convert the humidity float to a string with no decimal points shown
  // and add the % symbol at the end
  String humidity_text = String(humidity, 0) + "%";

  // Print each of the sensor values to the Serial Monitor
  Serial.print(temperature_text + " ");
  Serial.println(humidity_text);

  // Display the data on the UNO R4 WiFi LED matrix
  // The text will scroll from right to left across the matrix

  matrix.beginDraw();
  // Set stroke color to white (full brightness)
  matrix.stroke(0xFFFFFFFF);
  // Set scroll speed (higher = slower, 75 is a comfortable reading speed)
  matrix.textScrollSpeed(75);
  // Set font to 5x7 pixel font
  matrix.textFont(Font_5x7);
  // Begin text at position (0, 1) with white color
  matrix.beginText(0, 1, 0xFFFFFF);
  // Print the temperature and humidity text with spaces for readability
  matrix.println(" " + temperature_text + " " + humidity_text + " ");
  // End text and set it to scroll left
  matrix.endText(SCROLL_LEFT);
  // Finish drawing and update the display
  matrix.endDraw();
}
