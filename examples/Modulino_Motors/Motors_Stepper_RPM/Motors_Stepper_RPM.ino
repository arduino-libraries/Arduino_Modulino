/*
 * Modulino Motors - Stepper RPM
 *
 * This example demonstrates stepper mode with RPM-based movement
 * and the difference between holding torque and releasing coils
 * after a move completes.
 * It uses the constructor that only specifies steps-per-revolution.
 *
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Uses default address and no hub port, with 200 full-steps/rev.
ModulinoMotors motors(200);

void setup() {
  Serial.begin(9600);
  Modulino.begin();
  motors.begin();

  motors.setStepperModeEnabled(true);
  motors.setHalfStepEnabled(false);  // full-step
  motors.setDecay(ModulinoMotors::DecayMode::FAST);
}

void waitUntilIdle() {
  while (true) {
    if (!motors.update()) {
      delay(5);
      continue;
    }
    if (!motors.busy()) {
      break;
    }
    delay(5);
  }
}

void runMove(const char* label, int32_t steps, float rpm, bool releaseOnComplete) {
  Serial.print(label);
  Serial.print(" | release_on_complete=");
  Serial.println(releaseOnComplete ? "true" : "false");
  motors.moveStepperRpm(steps, rpm, releaseOnComplete);
  waitUntilIdle();
  delay(600);
}

void loop() {
  runMove("Forward: 1 rev at 60 RPM, hold at target", 200, 60.0f, false);

  runMove("Backward: 1 rev at 120 RPM, release at target", -200, 120.0f, true);

  Serial.println("Half-step mode: hold, then release");
  motors.setHalfStepEnabled(true);  // effective steps/rev doubles to 400
  runMove("Half-step forward: 1 rev at 90 RPM", 400, 90.0f, false);
  runMove("Half-step backward: 1 rev at 45 RPM", -400, 45.0f, true);

  motors.setHalfStepEnabled(false);
  delay(1200);
}
