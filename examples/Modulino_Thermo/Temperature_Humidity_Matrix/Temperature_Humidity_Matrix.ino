/*
 * Modulino Thermo - Temperature Humidity Matrix
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

float temperature = -273.15;
float humidity = 0.0;

void setup() {
  Serial.begin(9600);

  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to temperature/humidity sensor module
  thermo.begin();

  // Initialize the LED matrix
  matrix.begin();
  delay(100);
}

void loop() {
  //Acquire temperature and humidity
  temperature = thermo.getTemperature();
  humidity = thermo.getHumidity();

  //Convert the temperature float to a string with 1 decimal point shown
  //and add °C at the end
  String temperature_text = String(temperature, 1) + "°C";

  //Convert the humidity float to a string with no decimal points shown
  //and add % at the end
  String humidity_text = String(humidity, 0) + "%";

  //Print each of the sensor values on serial
  Serial.print(temperature_text + " ");
  Serial.println(humidity_text);

  //Show on the UNO R4 WiFi LED matrix the data

  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textScrollSpeed(75);
  matrix.textFont(Font_5x7);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(" " + temperature_text + " " + humidity_text + " ");
  matrix.endText(SCROLL_LEFT);
  matrix.endDraw();
}
