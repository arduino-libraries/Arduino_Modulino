/*
 * Modulino Thermo - Basic
 * 
 * This example demonstrates how to read temperature and humidity measurements
 * from the Modulino Thermo sensor.
 * 
 * The sensor provides:
 * - Temperature: Measured in degrees Celsius (°C)
 *   Typical range: -40°C to +85°C
 *   Accuracy: ±0.2°C
 * 
 * - Humidity: Measured in relative humidity percentage (rH%)
 *   Range: 0% to 100%
 *   Accuracy: ±2%
 * 
 * This example also demonstrates converting temperature from Celsius to Fahrenheit
 * using the formula: F = (C × 9/5) + 32
 * 
 * Applications:
 * - Weather stations
 * - Climate control
 * - Environmental monitoring
 * - Greenhouse automation
 * - HVAC systems
 *
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Create object instance
ModulinoThermo thermo;

void setup(){
  Serial.begin(9600);

  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to temperature/humidity sensor module
  thermo.begin();
}

void loop(){
  // Read temperature in Celsius from the sensor
  float celsius = thermo.getTemperature();

  // Convert Celsius to Fahrenheit
  // Formula: F = (C × 9/5) + 32
  float fahrenheit = (celsius * 9 / 5) + 32;

  // Read relative humidity percentage from the sensor
  // Returns value from 0.0 to 100.0
  float humidity = thermo.getHumidity();

  // Print temperature in Celsius
  Serial.print("Temperature (C) is: ");
  Serial.println(celsius);

  // Print temperature in Fahrenheit
  Serial.print("Temperature (F) is: ");
  Serial.println(fahrenheit);

  // Print relative humidity percentage
  Serial.print("Humidity (rH) is: ");
  Serial.println(humidity);

}
