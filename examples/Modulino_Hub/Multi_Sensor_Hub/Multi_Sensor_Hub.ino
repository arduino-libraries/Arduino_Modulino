/*
 * Modulino Multi Sensor - Hub Example
 * 
 * This example demonstrates how to use multiple Modulino sensors of the same type
 * by using the Modulino Hub (I2C multiplexer). Because devices of the same type
 * share the same I2C address, they would normally conflict. The hub solves this
 * by assigning each one to a different port.
 * 
 * In this example, we connect:
 * - Thermo sensor A and Movement sensor A on Hub port 0
 * - Thermo sensor B and Movement sensor B on Hub port 1
 *
 * This example code is in the public domain. 
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Create a ModulinoHub object
ModulinoHub hub;

constexpr int HUB_PORT_A = 0; // Port number on the Hub for first set of sensors
constexpr int HUB_PORT_B = 1; // Port number on the Hub for second set of sensors

// Create ModulinoThermo and ModulinoMovement objects on different ports
ModulinoThermo thermoA(hub.port(HUB_PORT_A));
ModulinoMovement movementA(hub.port(HUB_PORT_A));

ModulinoThermo thermoB(hub.port(HUB_PORT_B));
ModulinoMovement movementB(hub.port(HUB_PORT_B));

void printThermo(ModulinoThermo& thermo, int port) {
  float t = thermo.getTemperature();
  float h = thermo.getHumidity();
  Serial.print("Thermo [Port ");
  Serial.print(port);
  Serial.print("] -> Temp: ");
  Serial.print(t);
  Serial.print("C, Hum: ");
  Serial.print(h);
  Serial.println("%");
}

void printMovement(ModulinoMovement& movement, int port) {
  movement.update();
  Serial.print("Movement [Port ");
  Serial.print(port);
  Serial.print("] -> A(x,y,z): ");
  Serial.print(movement.getX(), 2); Serial.print(", ");
  Serial.print(movement.getY(), 2); Serial.print(", ");
  Serial.print(movement.getZ(), 2);
  Serial.print(" | G(r,p,y): ");
  Serial.print(movement.getRoll(), 1); Serial.print(", ");
  Serial.print(movement.getPitch(), 1); Serial.print(", ");
  Serial.println(movement.getYaw(), 1);
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // Initialize Modulino I2C communication
  Modulino.begin();
  
  // Detect and connect to sensors via the hub
  if (!thermoA.begin()) {
    Serial.print("Thermo A not found.");
  }
  
  if (!thermoB.begin()) {
    Serial.print("Thermo B not found.");
  }

  if (!movementA.begin()) {
    Serial.print("Movement A not found.");
  }

  if (!movementB.begin()) {
    Serial.print("Movement B not found.");
  }
}

void loop() {
  // Read and print from Thermo A
  printThermo(thermoA, HUB_PORT_A);

  // Read and print from Thermo B
  printThermo(thermoB, HUB_PORT_B);

  // Read and print from Movement A
  printMovement(movementA, HUB_PORT_A);

  // Read and print from Movement B
  printMovement(movementB, HUB_PORT_B);

  Serial.println("-------------------------------------------------");
  delay(1000);
}
