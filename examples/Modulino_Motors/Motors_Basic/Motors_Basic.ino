/*
 * Modulino Motors - Basic
 *
 * This example demonstrates basic DC motor control using ModulinoMotors.
 * It ramps motor A forward/reverse, then motor B, and finally stops both.
 *
 * This example code is in the public domain.
 * Copyright (C) Arduino s.r.l. and/or its affiliated companies
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

ModulinoMotors motors;

void setup() {
  Serial.begin(9600);
  Modulino.begin();
  motors.begin();

  motors.setStepperModeEnabled(false);  // DC mode

  // The decay mode defines what happens when the motor is commanded to stop or change speed/direction:
  // - SLOW decay mode allows current to decrease gradually, which can provide smoother stops and better low-speed torque, but may cause more heat and less efficient braking.
  // - FAST decay mode allows current to decrease rapidly (reverses H-bridge), which can provide quicker stops and more efficient braking, but may cause more audible noise and less torque at low speeds.
  // - Mixed decay modes provide a balance between the two by using a combination of fast and slow decay.
  motors.setDecay(ModulinoMotors::DecayMode::SLOW);
}

void loop() {
  Serial.println("Motor A forward");
  motors.setInvertA(false);
  motors.setSpeedA(55); // Set speed to 55% of full scale
  motors.setSpeedB(0); // Ensure motor B is stopped
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
