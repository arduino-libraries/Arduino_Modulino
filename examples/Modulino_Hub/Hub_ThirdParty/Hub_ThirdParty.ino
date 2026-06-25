/*
 * Modulino Hub - Third Party LSM6DSOX Example
 *
 * This example shows how to use the Modulino Hub to connect and communicate
 * with a 3rd party I2C module (an LSM6DSOX IMU).
 *
 * When using 3rd party modules, you are responsible for calling select(port)
 * and clear() manually on the hub to open and close the corresponding I2C channel.
 *
 * This example code is in the public domain. 
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>
#include <Arduino_LSM6DSOX.h>

ModulinoHub hub;

void setup() {
  Serial.begin(115200);
  
  // Initialize Modulino I2C communication
  Modulino.begin();

  // Route I2C traffic to port 0 on the Modulino Hub
  hub.select(0);

  // Initialize the 3rd party sensor
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  
  // Clear the hub routing to free the I2C bus for other ports
  hub.clear();
}

void loop() {
  float x, y, z;

  // Re-select the port before reading data from the sensor
  hub.select(0);
  
  bool dataReady = false;
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
    dataReady = true;
  }

  // Clear the port selection immediately after communicating
  hub.clear();

  if (dataReady) {
    Serial.print("Accel X: ");
    Serial.print(x);
    Serial.print("\tY: ");
    Serial.print(y);
    Serial.print("\tZ: ");
    Serial.println(z);
  }

  delay(100);
}
