/*
 * Modulino Motors - Basic
 *
 * This example demonstrates basic DC motor control using ModulinoMotors.
 * It ramps motor A forward/reverse, then motor B, and finally stops both.
 *
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

ModulinoMotors motors;

void setup() {
  Serial.begin(9600);
  Modulino.begin();
  motors.begin();

  motors.setStepperModeEnabled(false);  // DC mode
  motors.setDecay(ModulinoMotors::DecayMode::SLOW);
}

void loop() {
  Serial.println("Motor A forward");
  motors.setInvertA(false);
  motors.setSpeedA(55);
  motors.setSpeedB(0);
  delay(1200);

  Serial.println("Motor A reverse");
  motors.setInvertA(true);
  motors.setSpeedA(55);
  delay(1200);

  motors.setSpeedA(0);
  delay(300);

  Serial.println("Motor B forward");
  motors.setInvertB(false);
  motors.setSpeedB(55);
  delay(1200);

  Serial.println("Motor B reverse");
  motors.setInvertB(true);
  motors.setSpeedB(55);
  delay(1200);

  Serial.println("Stop");
  motors.stop();
  delay(1500);
}
