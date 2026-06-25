/*
 * Modulino Motors - Frequency
 *
 * This example demonstrates changing DC PWM frequency while the motors run
 * at low speed, which can alter audible motor tone and behavior.
 *
 * This example code is in the public domain.
 * Copyright (C) Arduino s.r.l. and/or its affiliated companies
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

ModulinoMotors motors;

constexpr uint8_t BASE_SPEED = 30;  // Keep low speed for easier listening/testing.
const uint16_t frequencies[] = {400, 800, 1200, 2000, 8000, 20000};

void setup() {
  Serial.begin(9600);
  Modulino.begin();
  motors.begin();

  motors.setStepperModeEnabled(false);  // DC mode
  motors.setDecay(ModulinoMotors::DecayMode::FAST);

  // Keep low speed for easier listening/testing.
  motors.setSpeedA(BASE_SPEED);
  motors.setSpeedB(BASE_SPEED);
}

void loop() {
  for (size_t i = 0; i < sizeof(frequencies) / sizeof(frequencies[0]); i++) {
    uint16_t f = frequencies[i];
    Serial.print("Set frequency: ");
    Serial.print(f);
    Serial.println(" Hz");
    motors.setFrequency(f);
    delay(1200);
  }

  motors.stop();
  delay(1500);

  // Restore firmware default for next cycle.
  motors.setFrequency(20000);
  motors.setSpeedA(BASE_SPEED);
  motors.setSpeedB(BASE_SPEED);
}
