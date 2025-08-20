/*
 * Modulino Thermo - Basic
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
  float fahrenheit = (celsius * 9 / 5) + 32;

  // Read humidity percentage from the sensor
  float humidity = thermo.getHumidity();

  Serial.print("Temperature (C) is: ");
  Serial.println(celsius);

  Serial.print("Temperature (F) is: ");
  Serial.println(fahrenheit);

  Serial.print("Humidity (rH) is: ");
  Serial.println(humidity);

}
