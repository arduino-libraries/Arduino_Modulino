/*
 * Modulino Motors - Telemetry
 *
 * This example demonstrates telemetry polling and current readout,
 * including HFS mode (Half-Full-Scale) effect on sensed current scaling.
 * Half-full-scale mode can be enabled to improve current accuracy in the lower range 
 * at the cost of maximum current capacity (1.9A MAX in HFS mode vs 3.8A MAX in full-scale mode).
 * 
 * This example code is in the public domain.
 * Copyright (C) Arduino s.r.l. and/or its affiliated companies
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

ModulinoMotors motors;
constexpr uint8_t BASE_SPEED = 50;

void printTelemetry(const char* label) {
  if (!motors.update()) {
    Serial.println("Telemetry read failed");
    return;
  }

  Serial.print(label);
  Serial.print(" | busy=");
  Serial.print(motors.busy() ? "true" : "false");
  Serial.print(" | mode=");
  Serial.print(motors.stepperModeEnabled() ? "stepper" : "dc");
  Serial.print(" | halfStep=");
  Serial.print(motors.halfStepEnabled() ? "true" : "false");
  Serial.print(" | decay=");
  Serial.print(static_cast<uint8_t>(motors.decayMode()));
  Serial.print(" | HFS=");
  Serial.print(motors.halfFullScaleEnabled() ? "true" : "false");
  Serial.print(" | I_A=");
  Serial.print(motors.sensedCurrentA(), 1);
  Serial.print(" mA | I_B=");
  Serial.print(motors.sensedCurrentB(), 1);
  Serial.println(" mA");
}

void setup() {
  Serial.begin(9600);
  Modulino.begin();
  motors.begin();

  motors.setStepperModeEnabled(false); // DC mode
  motors.setDecay(ModulinoMotors::DecayMode::FAST);
  motors.setSpeedA(BASE_SPEED);
  motors.setSpeedB(BASE_SPEED);
}

void loop() {
  motors.setHalfFullScaleEnabled(false);
  delay(200); // Allow time for mode change to take effect before reading telemetry
  // Print results using full scale current sensing
  printTelemetry("Full-scale");

  motors.setHalfFullScaleEnabled(true);
  delay(200); // Allow time for mode change to take effect before reading telemetry
  // Print results using half-full-scale current sensing
  printTelemetry("Half-scale");

  motors.stop();
  delay(300); // Allow time for stop command to take effect before reading telemetry
  printTelemetry("After stop");

  motors.setHalfFullScaleEnabled(false);
  motors.setSpeedA(BASE_SPEED);
  motors.setSpeedB(BASE_SPEED);
  delay(1500);
}
