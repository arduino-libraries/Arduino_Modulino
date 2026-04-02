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
      delay(10);
      continue;
    }
    if (!motors.busy()) {
      break;
    }
    delay(10);
  }
}

void runMove(const char* label, int32_t steps, float rpm, uint8_t releaseDelayMs) {
  Serial.print(label);
  Serial.print(" | release_delay_ms=");
  Serial.println(releaseDelayMs);
  motors.moveStepperRpm(steps, rpm, releaseDelayMs);
  waitUntilIdle();  
}

void loop() {
  runMove("Forward: 1 rev at 60 RPM, hold at target", 200, 60.0f, 0);
  delay(600);

  runMove("Backward: 1 rev at 120 RPM, release after 50ms", -200, 120.0f, 50);
  delay(600);

  Serial.println("Half-step mode: hold, then release");
  motors.setHalfStepEnabled(true);  // effective steps/rev doubles to 400
  runMove("Half-step forward: 1 rev at 90 RPM", 400, 90.0f, 0);
  delay(600);
  runMove("Half-step backward: 1 rev at 45 RPM", -400, 45.0f, 50);
  delay(600);

  motors.setHalfStepEnabled(false);
  delay(1200);
}
