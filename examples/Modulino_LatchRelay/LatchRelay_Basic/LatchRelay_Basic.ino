/*
 * Modulino Latch Relay - Basic
 *
 * This example code is in the public domain. 
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Modulino.h>

ModulinoLatchRelay relay;

void setup() {
  // put your setup code here, to run once:
  Modulino.begin();
  Serial.begin(115200);
  relay.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()) {
    char c = Serial.read();
    switch (c) {
      case 's':
        relay.set();
        break;
      case 'r':
        relay.reset();
        break;
      case 'x':
        auto status = relay.getStatus();
        if (status == 0) {
          Serial.println("Relay OFF");
        }
        if (status == 1) {
          Serial.println("Relay ON");
        }
        if (status < 0) {
          Serial.println("Relay status unknown");
        }
        break;
    }
  }
}
