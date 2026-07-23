/*
 * Modulino - Firmware Updater
 * 
 * This utility updates the firmware on Modulino modules.
 * 
 * IMPORTANT: This is an advanced tool for updating module firmware.
 * Only use this if instructed by Arduino support or if you need to
 * restore a module to working condition.
 * 
 * Instructions:
 * 1. Connect ONLY ONE Modulino module at a time
 * 2. Upload this sketch to your Arduino
 * 3. The sketch will automatically detect and flash the appropriate firmware
 * 4. On UNO R4 WiFi, the LED matrix will show "PASS" or "FAIL" when done
 * 5. Wait for the update to complete before disconnecting
 * 
 * Special case for LED Matrix:
 * - Connect only the LED Matrix and answer "y" when prompted
 * 
 * NOTE: This uses the STM32 bootloader protocol to flash firmware.
 * Do not disconnect power during the update process.
 * 
 * Reference: STM32 I2C bootloader protocol
 * https://www.st.com/resource/en/application_note/an4221-i2c-protocol-used-in-the-stm32-bootloader-stmicroelectronics.pdf
 *
 * This example code is in the public domain. 
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#if defined(ARDUINO_UNOWIFIR4)
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#endif

#include <Arduino_Modulino.h>
#include <ModulinoScanner.h>
#include <ModulinoFlasher.h>
#include "fw.h"
#include "fw_ledmatrix.h"
#include <fw_motors.h>

// --- Firmware Registry ---

struct FirmwareDescriptor {
  ModulinoFirmware type;
  const char* name;
  const unsigned char* binData;
  unsigned int binLen;
};

const FirmwareDescriptor FIRMWARES[] = {
  { FIRMWARE_GENERIC,    "Generic (Buzzer, Buttons, Encoder, ...)", node_base_bin,        node_base_bin_len },
  { FIRMWARE_MOTORS,     "Motors",                                  motors_node_base_bin, motors_node_base_bin_len },
  { FIRMWARE_LED_MATRIX, "LED Matrix",                              matrix_node_base_bin, matrix_node_base_bin_len }
};
const size_t NUM_FIRMWARES = sizeof(FIRMWARES) / sizeof(FIRMWARES[0]);

// --- Objects ---
Module modulino;
ModulinoScanner scanner;
ModulinoFlasher flasher(modulino);

// --- State machine ---
enum State {
  INIT,
  SCAN_DEVICES,
  WAIT_USER_CHOICE,
  WAIT_BOOT_MODE_CHOICE,
  FLASHING,
  DONE
};

State state = INIT;
int selectedAddress  = -1;
ModulinoFirmware firmwareType = FIRMWARE_GENERIC;

// --- UNO R4 WiFi LED matrix helpers ---

#if defined(ARDUINO_UNOWIFIR4)
ArduinoLEDMatrix matrix;

/**
 * @brief Renders a progress bar on the UNO R4 WiFi LED matrix.
 *
 * Lights a proportional number of the 96 LEDs (8 rows x 12 columns),
 * filling left-to-right, top-to-bottom.
 *
 * @param progress Number of completed pages.
 * @param total    Total number of pages.
 */
void updateProgressMatrix(int progress, int total) {
  uint32_t frame[3] = {0, 0, 0};
  int numLeds = (progress * 96) / total;
  for (int i = 0; i < numLeds; i++) {
    int row = i / 12;
    int col = 11 - (i % 12);
    frame[row / 4] |= (1 << (col + (3 - (row % 4)) * 12));
  }
  matrix.loadFrame(frame);
}

/**
 * @brief Initialises the UNO R4 WiFi LED matrix and renders a short status string.
 *
 * Uses the 4x6 pixel font. Intended for "PASS" or "FAIL" messages.
 * @param text Null-terminated string to display.
 */
void matrixInitAndDraw(const char* text) {
  matrix.begin();
  matrix.beginDraw();
  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText();
  matrix.endDraw();
}
#else
void updateProgressMatrix(int progress, int total) {}
void matrixInitAndDraw(const char* text) {}
#endif

// --- State Handlers ---

void handleInit() {
  if (Serial) {
    state = SCAN_DEVICES;
  }
}

void handleScanDevices() {
  // Check whether a device is already waiting in bootloader mode.
  if (scanner.deviceInBootloaderMode()) {
    Serial.println("A Modulino device was detected in bootloader mode.");
    Serial.println("Which firmware to flash?");
    for (size_t i = 0; i < NUM_FIRMWARES; i++) {
      Serial.print("  [");
      Serial.print(i + 1);
      Serial.print("] ");
      Serial.println(FIRMWARES[i].name);
    }
    state = WAIT_BOOT_MODE_CHOICE;
  } else {
    Serial.println("\nScanning for connected Modulino devices...");
    scanner.discover();

    if (scanner.numDevices == 0) {
      Serial.println("No configurable Modulinos found.");
      delay(2000);
      state = SCAN_DEVICES;
    } else {
      scanner.printDevices(true);
      if (scanner.numDevices == 1) {
        Serial.println("\nProceed with flashing this device? [y/n]");
      } else {
        Serial.print("\nEnter the number of the Modulino to flash (1-");
        Serial.print(scanner.numDevices);
        Serial.println("):");
      }
      state = WAIT_USER_CHOICE;
    }
  }
}

void handleWaitBootModeChoice() {
  if (Serial.available() > 0) {
    String inputStr = Serial.readStringUntil('\n');
    inputStr.trim();
    if (inputStr.length() == 0) return;
    
    int choice = inputStr.toInt();
    bool found = false;
    
    // Check if the choice is valid (1 to NUM_FIRMWARES)
    if (choice >= 1 && choice <= (int)NUM_FIRMWARES) {
      firmwareType = FIRMWARES[choice - 1].type;
      found = true;
    }
    
    if (!found) {
      // Fallback to first firmware (generic) if input is invalid but not empty
      firmwareType = FIRMWARES[0].type;
    }
    state = FLASHING;
  }
}

void handleWaitUserChoice() {
  if (Serial.available() > 0) {
    String inputStr = Serial.readStringUntil('\n');
    inputStr.trim();
    if (inputStr.length() == 0) return;

    const DetectedModulino* device = nullptr;

    if (scanner.numDevices == 1) {
      if (inputStr != "y" && inputStr != "Y") {
        Serial.println("Aborted. Rescanning...");
        delay(1000);
        state = SCAN_DEVICES;
        return;
      }
      device = &scanner.devices[0];
    } else {
      int index = inputStr.toInt() - 1;
      if (index < 0 || index >= scanner.numDevices) {
        Serial.print("Invalid selection. Enter a number between 1 and ");
        Serial.print(scanner.numDevices);
        Serial.println(":");
        return;
      }
      device = &scanner.devices[index];
    }

    selectedAddress = device->addr;
    Serial.print("Sending reset to device 0x");
    Serial.println(selectedAddress, HEX);

    // Look up firmware type directly from the device registry.
    firmwareType = modulinoFirmwareForPinstrap(device->pinstrapValue);

    if (flasher.enterBootloader((uint8_t)selectedAddress)) {
      delay(250);
      state = FLASHING;
    } else {
      Serial.println("Failed to send reset command to the device.");
      state = SCAN_DEVICES;
    }
  }
}

void handleFlashing() {
  // The STM32 bootloader takes ~6500 ms to reboot after being contacted.
  // Wait for the remaining time so the flash commands arrive when it is ready.
  unsigned long waitMs = scanner.bootloaderReadyIn();
  if (waitMs > 0) {
    Serial.print("Waiting for bootloader to be ready (");
    Serial.print(waitMs);
    Serial.println(" ms)...");
    delay(waitMs);
  }

  Serial.println("\nStarting firmware update...");

  const FirmwareDescriptor* fw = nullptr;
  for (size_t i = 0; i < NUM_FIRMWARES; i++) {
    if (FIRMWARES[i].type == firmwareType) {
      fw = &FIRMWARES[i];
      break;
    }
  }

  // Fallback to first firmware if type somehow not found
  if (fw == nullptr) fw = &FIRMWARES[0];

  Serial.print("Flashing Modulino ");
  Serial.print(fw->name);
  Serial.println(" Firmware...");

  bool result = flasher.flash(fw->binData, fw->binLen, false, updateProgressMatrix);

  if (result) {
    Serial.println("\nFirmware update completed SUCCESSFULLY!");
    matrixInitAndDraw("PASS");
  } else {
    Serial.println("\nFirmware update FAILED.");
    matrixInitAndDraw("FAIL");
  }
  state = DONE;
}

void handleDone() {
  /* Wait for the user to reset the board. */
  delay(1000);
}

// --- Arduino entry points ---

/**
 * @brief Arduino setup routine.
 *
 * Initialises Serial, the Modulino I2C bus at 400 kHz, and (on UNO R4 WiFi)
 * the on-board LED matrix. Sets the state machine to INIT.
 */
void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(100);  // allow Serial to stabilise
  Modulino.begin();
  scanner.begin(*modulino.getWire());

#if defined(ARDUINO_UNOWIFIR4)
  matrix.begin();
#endif

  state = INIT;
}

/**
 * @brief Arduino main loop - drives the firmware-update state machine.
 */
void loop() {
  switch (state) {
    case INIT:                  handleInit(); break;
    case SCAN_DEVICES:          handleScanDevices(); break;
    case WAIT_BOOT_MODE_CHOICE: handleWaitBootModeChoice(); break;
    case WAIT_USER_CHOICE:      handleWaitUserChoice(); break;
    case FLASHING:              handleFlashing(); break;
    case DONE:                  handleDone(); break;
  }
}
