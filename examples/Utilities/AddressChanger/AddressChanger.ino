/*
 * Modulino - Address Changer
 *
 * This utility allows you to change the I2C addresses of Modulino modules.
 * This is essential when you want to use multiple modules of the same type
 * on the same I2C bus (e.g., multiple encoders or buttons).
 *
 * By default, each Modulino type has a fixed default I2C address. If you connect
 * two modules of the same type, they will conflict. This tool lets you change
 * the address to avoid conflicts.
 *
 * How to use:
 * 1. Connect the Arduino and open the Serial Monitor (115200 baud)
 * 2. The tool lists all detected Modulino devices with an index number
 * 3. Enter the index of the device whose address you want to change,
 *    or 0 to reset ALL connected devices to their default address
 * 4. Enter the new address in hex (e.g. "0x30"), or 0 to reset that
 *    device to its default address
 * 5. The tool re-reads the pinstrap byte at the new address to confirm
 *    the change was applied to the right device
 *
 * IMPORTANT NOTES:
 * - Valid I2C addresses range from 0x08 to 0x77
 * - Some devices have fixed addresses and cannot be changed (Distance, Thermo, Movement)
 * - The new address is stored in the module's memory permanently
 * - After changing addresses, power cycle the modules to ensure changes take effect
 *
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>
#include <ModulinoScanner.h>

// --- I2C addressing ---
constexpr uint8_t MIN_CONFIGURABLE_ADDRESS = 0x08;  ///< Lowest address accepted by module firmware.
constexpr uint8_t MAX_CONFIGURABLE_ADDRESS = 0x77;  ///< Highest address accepted by module firmware.
constexpr uint8_t USE_DEFAULT_ADDRESS      = 0x00;  ///< Value the user enters to mean "use the default address".
constexpr uint8_t BROADCAST_ADDRESS        = 0x00;  ///< I2C address every configurable module listens on.

/** @brief Converts a pinstrap byte to the default I2C address it encodes (pinstrap == address * 2). */
constexpr uint8_t pinstrapToDefaultAddress(uint8_t pinstrap) { return pinstrap / 2; }
/** @brief Converts an I2C address to the pinstrap-style value the change-address command expects. */
constexpr uint8_t addressToPinstrap(uint8_t address) { return address * 2; }

// --- Change-address I2C command ---
// Modules recognize a "CF" header followed by the new address encoded in
// pinstrap form, zero-padded to fill a fixed-length frame.
constexpr uint8_t ADDRESS_CHANGE_CMD_HEADER[2]        = { 'C', 'F' };
constexpr size_t  ADDRESS_CHANGE_CMD_FRAME_LEN         = 40;
constexpr unsigned long ADDRESS_CHANGE_APPLY_DELAY_MS = 500;  ///< Time for a module to store and apply its new address.

// --- Serial / scan timing ---
constexpr unsigned long SERIAL_BAUD_RATE          = 115200;
constexpr unsigned long SERIAL_STABILIZE_DELAY_MS = 100;
constexpr unsigned long NO_DEVICES_RETRY_DELAY_MS = 2000;

// --- Objects ---
Module modulino;
ModulinoScanner scanner;

// --- State machine ---
enum State {
  INIT,
  SCAN_DEVICES,
  WAIT_DEVICE_CHOICE,
  WAIT_BROADCAST_CONFIRM,
  WAIT_NEW_ADDRESS,
  APPLY_ADDRESS_CHANGE
};

State state = INIT;

int selectedDeviceIndex = -1;  ///< Index into scanner.devices for the device being changed, or -1 while broadcasting.
uint8_t sourceAddress = 0;     ///< Current address of the device being changed (or BROADCAST_ADDRESS).
uint8_t targetAddress = 0;     ///< New address to apply.
uint8_t expectedPinstrap = 0;  ///< Pinstrap byte expected at targetAddress after a successful change, 0 to skip verification.

// --- State handlers ---

void handleInit() {
  if (Serial) state = SCAN_DEVICES;
}

void handleScanDevices() {
  Serial.println("\nScanning for connected Modulino devices...");
  scanner.discover();

  if (scanner.numDevices == 0) {
    Serial.println("No Modulino devices found.");
    delay(NO_DEVICES_RETRY_DELAY_MS);
    return;  // Stay in SCAN_DEVICES and try again.
  }

  scanner.printDevices(true);
  Serial.print("\nEnter the number of the Modulino to change (1-");
  Serial.print(scanner.numDevices);
  Serial.println("), or 0 to reset ALL devices to their default address:");

  state = WAIT_DEVICE_CHOICE;
}

void handleWaitDeviceChoice() {
  if (Serial.available() == 0) return;

  String input = Serial.readStringUntil('\n');
  input.trim();
  if (input.length() == 0) return;

  if (!isDecimalNumber(input)) {
    Serial.print("Invalid input. Enter a number between 0 and ");
    Serial.print(scanner.numDevices);
    Serial.println(":");
    return;
  }

  int choice = input.toInt();

  if (choice == 0) {
    Serial.println("This will reset ALL connected devices to their default address. Proceed? [y/n]");
    state = WAIT_BROADCAST_CONFIRM;
    return;
  }

  if (choice < 1 || choice > scanner.numDevices) {
    Serial.print("Invalid selection. Enter a number between 0 and ");
    Serial.print(scanner.numDevices);
    Serial.println(":");
    return;
  }

  const DetectedModulino& device = scanner.devices[choice - 1];
  if (device.isFixed) {
    Serial.println("That device has a fixed address and cannot be changed. Choose another:");
    return;
  }

  selectedDeviceIndex = choice - 1;
  sourceAddress = device.addr;

  Serial.print("Selected ");
  Serial.print(device.modulinoType);
  Serial.print(" at 0x");
  Serial.println(device.addr, HEX);
  Serial.print("Enter the new address in hex (e.g. \"0x30\"), or 0 to reset to its default address (0x");
  Serial.print(pinstrapToDefaultAddress(device.pinstrapValue), HEX);
  Serial.println("):");

  state = WAIT_NEW_ADDRESS;
}

void handleWaitBroadcastConfirm() {
  if (Serial.available() == 0) return;

  String input = Serial.readStringUntil('\n');
  input.trim();
  if (input.length() == 0) return;

  if (input.equalsIgnoreCase("y")) {
    selectedDeviceIndex = -1;
    sourceAddress = BROADCAST_ADDRESS;
    targetAddress = USE_DEFAULT_ADDRESS;
    expectedPinstrap = 0;  // Multiple devices reset at once; nothing to verify.
    state = APPLY_ADDRESS_CHANGE;
  } else {
    Serial.println("Cancelled.");
    state = SCAN_DEVICES;
  }
}

void handleWaitNewAddress() {
  if (Serial.available() == 0) return;

  String input = Serial.readStringUntil('\n');
  int newAddress = parseHexAddress(input);

  if (newAddress == -1) {
    Serial.println("Error: enter a valid hexadecimal address, or 0 for the default. Try again:");
    return;
  }

  if (newAddress != USE_DEFAULT_ADDRESS && (newAddress < MIN_CONFIGURABLE_ADDRESS || newAddress > MAX_CONFIGURABLE_ADDRESS)) {
    Serial.print("Error: address must be between 0x");
    Serial.print(MIN_CONFIGURABLE_ADDRESS, HEX);
    Serial.print(" and 0x");
    Serial.print(MAX_CONFIGURABLE_ADDRESS, HEX);
    Serial.println(". Try again:");
    return;
  }

  const DetectedModulino& device = scanner.devices[selectedDeviceIndex];
  targetAddress = (newAddress == USE_DEFAULT_ADDRESS) ? pinstrapToDefaultAddress(device.pinstrapValue) : (uint8_t)newAddress;
  expectedPinstrap = device.pinstrapValue;

  state = APPLY_ADDRESS_CHANGE;
}

void handleApplyAddressChange() {
  bool isBroadcast = (selectedDeviceIndex == -1);

  Serial.print("Updating ");
  Serial.print(isBroadcast ? "all devices (broadcast)" : "the device");
  Serial.print(" to address 0x");
  Serial.print(targetAddress, HEX);
  Serial.print("...");

  sendAddressChangeCommand(sourceAddress, targetAddress);
  delay(ADDRESS_CHANGE_APPLY_DELAY_MS);

  if (isBroadcast) {
    Serial.println(" done");
  } else if (confirmAddressChange(targetAddress, expectedPinstrap)) {
    Serial.println(" done, verified");
  } else {
    Serial.println(" failed: could not confirm the device at the new address");
  }

  selectedDeviceIndex = -1;
  state = SCAN_DEVICES;
}

// --- I2C helpers ---

// Sends the "CF" change-address command to currentAddress, telling the module to
// switch to newAddress (or reset to its default address when newAddress is 0).
void sendAddressChangeCommand(uint8_t currentAddress, uint8_t newAddress) {
  uint8_t frame[ADDRESS_CHANGE_CMD_FRAME_LEN] = {
    ADDRESS_CHANGE_CMD_HEADER[0],
    ADDRESS_CHANGE_CMD_HEADER[1],
    addressToPinstrap(newAddress)
  };
  memset(frame + 3, 0, sizeof(frame) - 3);  // Zero the rest of the frame.

  modulino.getWire()->beginTransmission(currentAddress);
  modulino.getWire()->write(frame, ADDRESS_CHANGE_CMD_FRAME_LEN);
  modulino.getWire()->endTransmission();
}

// Reads the pinstrap byte back from newAddress and checks it matches the device
// that was there before the change, confirming the right module moved.
bool confirmAddressChange(uint8_t newAddress, uint8_t expectedPinstrap) {
  modulino.getWire()->requestFrom(newAddress, (uint8_t)1);
  if (!modulino.getWire()->available()) return false;
  return modulino.getWire()->read() == expectedPinstrap;
}

// --- Input parsing helpers ---

bool isDecimalNumber(const String& str) {
  for (unsigned int i = 0; i < str.length(); i++) {
    if (!isDigit(str.charAt(i))) return false;
  }
  return true;
}

bool isHexadecimalChar(char c) {
  return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}

// Parses a hex address string (with or without a "0x" prefix). Returns -1 if invalid.
int parseHexAddress(String hexStr) {
  hexStr.trim();
  if (hexStr.length() == 0) return -1;

  if (hexStr.startsWith("0x") || hexStr.startsWith("0X")) {
    hexStr = hexStr.substring(2);
  }

  for (unsigned int i = 0; i < hexStr.length(); i++) {
    if (!isHexadecimalChar(hexStr.charAt(i))) return -1;
  }

  return strtol(hexStr.c_str(), NULL, 16);
}

// --- Arduino entry points ---

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial) {}
  delay(SERIAL_STABILIZE_DELAY_MS);  // Allow serial port to stabilize.

  Modulino.begin();
  scanner.begin(*modulino.getWire());

  state = INIT;
}

void loop() {
  switch (state) {
    case INIT:                   handleInit(); break;
    case SCAN_DEVICES:           handleScanDevices(); break;
    case WAIT_DEVICE_CHOICE:     handleWaitDeviceChoice(); break;
    case WAIT_BROADCAST_CONFIRM: handleWaitBroadcastConfirm(); break;
    case WAIT_NEW_ADDRESS:       handleWaitNewAddress(); break;
    case APPLY_ADDRESS_CHANGE:   handleApplyAddressChange(); break;
  }
}
