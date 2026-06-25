/*
 * Modulino Motors - Stepper RPM
 *
 * This example demonstrates stepper mode with RPM-based movement
 * and the difference between holding torque and releasing coils
 * after a move completes.
 * It uses the constructor that only specifies steps-per-revolution.
 *
 * This example code is in the public domain.
 * Copyright (C) Arduino s.r.l. and/or its affiliated companies
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
  // Half stepping provides smoother motion by using twice as many steps per revolution, 
  // but it also reduces the maximum speed and holding torque
  motors.setHalfStepEnabled(false);  // full-step
  motors.setDecay(ModulinoMotors::DecayMode::FAST);
}

/**
 * Wait until the motors are no longer busy (i.e. a move has completed).
 * This is a simple polling loop that checks the busy status every 10 ms.
 * In a real application, you might want to add a timeout or handle other tasks while waiting
 */
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

/**
 * Run a stepper move with the specified parameters and print the label.
 * @param label A descriptive label for the move being executed.
 * @param steps The number of steps to move (positive for forward, negative for backward).
 * @param rpm The speed of the move in revolutions per minute.
 * @param releaseDelayMs The delay in milliseconds after the move completes before releasing the coils (0 to hold, > 0 to release after delay).
 */
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
