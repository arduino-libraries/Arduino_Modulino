/*
 * Modulino Latch Relay - Basic
 * 
 * This example demonstrates how to control the Modulino Latch Relay module via serial commands.
 * A latch relay maintains its state (ON or OFF) even when power is removed, making it
 * energy-efficient for applications that need to maintain a state.
 * 
 * Serial Commands:
 * - 's': Set the relay to ON position (energize/close the relay)
 * - 'r': Reset the relay to OFF position (de-energize/open the relay)
 * - 'x': Check and display the current relay status
 * 
 * Relay Status Values:
 * - 0: Relay is OFF (open circuit)
 * - 1: Relay is ON (closed circuit)
 * - Negative value: Status unknown or error
 * 
 * Usage:
 * 1. Open the Serial Monitor (set to 115200 baud)
 * 2. Type 's' and press Enter to turn the relay ON
 * 3. Type 'r' and press Enter to turn the relay OFF
 * 4. Type 'x' and press Enter to check the current status
 *
 * This example code is in the public domain. 
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Modulino.h>

// Create a ModulinoLatchRelay object
ModulinoLatchRelay relay;

void setup() {
  // Initialize Modulino I2C communication
  Modulino.begin();
  // Initialize serial communication at 115200 baud
  Serial.begin(115200);
  // Detect and connect to latch relay module
  relay.begin();
}

void loop() {
  // Check if data is available from the Serial Monitor
  if (Serial.available()) {
    // Read the incoming character command
    char c = Serial.read();
    switch (c) {
      case 's':
        // Set relay to ON position (energize the relay)
        relay.set();
        break;
      case 'r':
        // Reset relay to OFF position (de-energize the relay)
        relay.reset();
        break;
      case 'x':
        // Get and display the current relay status
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
