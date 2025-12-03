/*
 * Modulino - Plug and Play
 *
 * This example code is in the public domain. 
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

// Create objects for all Modulino modules
ModulinoButtons buttons;
ModulinoBuzzer buzzer;
ModulinoPixels leds;
ModulinoKnob encoder;
ModulinoDistance distance;
ModulinoMovement imu;
ModulinoThermo thermo;
ModulinoJoystick joystick;

void setup() {

  Serial.begin(115200);
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to all modules
  distance.begin();
  buttons.begin();
  encoder.begin();
  buzzer.begin();
  leds.begin();
  imu.begin();
  thermo.begin();
  joystick.begin();
}

int skip = 0;
int pitch = 0;
bool a = false;
bool b = false;
bool c = false;

void loop() {

  float x;
  float y;
  float z;
  // Cycle through LED patterns when encoder is pressed
  if (encoder.isPressed()) {
    skip = (skip + 1) % 5;
  }

  // Calculate pitch by combining encoder position and distance sensor value
  pitch = encoder.get() + (distance.available() ? distance.get() : 0);

  // When 's' is received over serial, report sensor data
  if (Serial.available() && Serial.read() == 's') {
    // Update and report accelerometer values
    imu.update();
    Serial.print("IMU: x ");
    Serial.print(imu.getX(), 3);
    Serial.print("\ty ");
    Serial.print(imu.getY(), 3);
    Serial.print("\tz ");
    Serial.println(imu.getZ(), 3);

    // Report temperature and humidity values
    Serial.print("Humidity: " + String(thermo.getHumidity()));
    Serial.println("\tTemperature: " + String(thermo.getTemperature()));
  }

  // Check for joystick movement
  if (joystick.update()) {
    Serial.println("x: " + String(joystick.getX()) + " // y: " + String(joystick.getY()) + " // pressed: " + String(joystick.isPressed()));
  }

  // Check for button presses
  if (buttons.update()) {
    // Button A: Red LED and 440Hz tone
    if (buttons.isPressed(0)) {
      leds.set(1 + skip, RED, 100);
      buzzer.tone(440 + pitch, 1000);
    } else {
      leds.clear(1 + skip);
    }
    // Button B: Blue LED and 880Hz tone
    if (buttons.isPressed(1)) {
      leds.set(2 + skip, BLUE, 100);
      buzzer.tone(880 + pitch, 1000);
    } else {
      leds.clear(2 + skip);
    }
    // Button C: Green LED and 1240Hz tone
    if (buttons.isPressed(2)) {
      leds.set(3 + skip, GREEN, 100);
      buzzer.tone(1240 + pitch, 1000);
    } else {
      leds.clear(3 + skip);
    }
    // Update the LEDs
    leds.show();
  }
}
