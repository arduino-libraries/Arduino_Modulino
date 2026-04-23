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
 * 2. The tool will show all detected Modulino devices with their addresses
 * 3. Enter commands in this format: "current_address new_address"
 *    Examples:
 *    - "0x3E 0x3F" - Changes device at 0x3E to address 0x3F
 *    - "0x3E 0" - Resets device at 0x3E to its default address
 *    - "0 0" - Resets ALL devices to their default addresses (broadcast)
 * 
 * IMPORTANT NOTES:
 * - Valid I2C addresses range from 0x08 to 0x77
 * - Some devices have fixed addresses and cannot be changed (Distance, Thermo, Movement)
 * - The new address is stored in the module's memory permanently
 * - After changing addresses, power cycle the modules to ensure changes take effect
 * 
 * Default addresses by module type:
 * - Buzzer: 0x1E (pinstrap 0x3C)
 * - Joystick: 0x2C (pinstrap 0x58)
 * - Buttons: 0x3E (pinstrap 0x7C)
 * - Opto Relay: 0x14 (pinstrap 0x28)
 * - Encoder: 0x3B or 0x3A (pinstrap 0x76 or 0x74)
 * - Smartleds: 0x36 (pinstrap 0x6C)
 * - Vibro: 0x38 (pinstrap 0x70)
 * - Distance: 0x29 (fixed, cannot change)
 * - Thermo: 0x44 (fixed, cannot change)
 * - Movement: 0x6A or 0x6B (fixed, cannot change)
 *
 * This example code is in the public domain. 
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include "Wire.h"

// Structure to store information about detected Modulino devices
struct DetectedModulino {
  uint8_t addr;           // Current I2C address
  String modulinoType;    // Type of module (e.g., "Buzzer", "Encoder")
  String pinstrap;        // Pinstrap value (identifies device type)
  String defaultAddr;     // Default address for this module type
};

#define MAX_DEVICES 16
DetectedModulino rows[MAX_DEVICES];  // Array to store detected devices
int numRows = 0;  // Number of devices currently detected


void setup() {
  // Initialize I2C communication on Wire1 interface
  Wire1.begin();
  // Initialize serial communication at 115200 baud
  Serial.begin(115200);

  // Wait for serial port to initialize
  while (!Serial) {};

  // Scan I2C bus and display all detected Modulino devices
  discoverDevices();
}

bool waitingInput = false;

void loop() {
  // If no devices detected, nothing to do
  if (numRows == 0) return;
  // If waiting for input and nothing available, keep waiting
  if (Serial.available() == 0 && waitingInput) return;

  // Process user input when available
  if (Serial.available() > 0) {
    // Read two hexadecimal values separated by space
    String hex1 = Serial.readStringUntil(' ');   // Current address
    String hex2 = Serial.readStringUntil('\n');  // New address
    // Echo back what user entered
    Serial.println("> " + hex1 + " " + hex2);

    // Parse the hexadecimal strings to integer values
    int num1 = parseHex(hex1);  // Current address
    int num2 = parseHex(hex2);  // New address
    
    // Validate input
    if (num1 == -1 || num2 == -1) {
      Serial.println("Error: Incomplete or invalid input. Please enter two hexadecimal numbers");
      return;
    }

    // Attempt to update the I2C address
    bool success = updateI2cAddress(num1, num2);
    if (!success) return;  // If update failed, wait for new input

    // Re-scan devices to show updated addresses
    discoverDevices();
    waitingInput = false;
  }

  // Display instructions for the user
  Serial.println("Enter the current address, space, and new address (ex. \"0x20 0x30\" or \"20 2A\"):");
  Serial.println("  - Enter \"<addr> 0\" to reset the device at <addr> to its default address.");
  Serial.println("  - Enter \"0 0\" to reset all devices to the default address.");
  waitingInput = true;
}

// Updates the device at current address to new address. Supports broadcasting and setting default address (0).
// Returns true if the update was successful, false otherwise.
bool updateI2cAddress(int curAddress, int newAddress) {
  uint8_t data[48] = { 'C', 'F', newAddress * 2 };
  memset(data + 3, 0, sizeof(data) - 3);  // Zero the rest of the buffer.

  // Validate the current address, it must match a detected device.
  if (curAddress != 0 && !findRow(curAddress)) {
    Serial.println("Error: current address 0x" + String(curAddress, HEX) + " not found in the devices list\n");
    return false;
  }

  if (curAddress != 0 && isFixedAddrDevice(curAddress)) {
    Serial.println("Error: address 0x" + String(curAddress, HEX) + " is a non configurable device\n");
    return false;
  }

  // Validate the new address.
  if (newAddress != 0 && (newAddress < 8 || newAddress > 0x77)) {
    Serial.println("Error: new address 0x" + String(newAddress, HEX) + " must be from 0x08 to 0x77\n");
    return false;
  }

  if (curAddress == 0) {
    Serial.print("Updating all devices (broadcast 0x00) to 0x" + String(newAddress, HEX));
  } else {
    Serial.print("Updating the device address from 0x" + String(curAddress, HEX) + " to 0x" + String(newAddress, HEX));
  }
  if (newAddress == 0) Serial.print(" (default address)");
  Serial.print("...");

  Wire1.beginTransmission(curAddress);
  Wire1.write(data, 40);
  Wire1.endTransmission();

  delay(500);

  if (newAddress == 0) {
    Serial.println(" done\n");
    return true;
  } else {
    Wire1.requestFrom(newAddress, 1);
    if (Wire1.available()) {
      Serial.println(" done\n");
      return true;
    } else {
      Serial.println(" error\n");
      return false;
    }
  }
}

// Function to parse hex number (with or without 0x prefix)
int parseHex(String hexStr) {
  hexStr.trim();

  if (hexStr.length() == 0) {
    return -1;
  }

  if (hexStr.startsWith("0x") || hexStr.startsWith("0X")) {
    hexStr = hexStr.substring(2);  // Remove the "0x" prefix
  }

  // Validate that the remaining string contains only valid hexadecimal characters (0-9, A-F, a-f)
  for (int i = 0; i < hexStr.length(); i++) {
    if (!isHexDigit(hexStr.charAt(i))) {
      return -1;
    }
  }

  return strtol(hexStr.c_str(), NULL, 16);
}

bool isHexDigit(char c) {
  return ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}

void discoverDevices() {
  char buffer[64];
  Serial.println("ADDR\tMODULINO\tPINSTRAP\tDEFAULT ADDR");  // Table heading.

  numRows = 0;

  // Discover all modulino devices connected to the I2C bus.
  for (int addr = 0; addr < 128; addr++) {
    Wire1.beginTransmission(addr);
    if (Wire1.endTransmission() != 0) continue;

    if (numRows >= MAX_DEVICES) {
      Serial.println("Too many devices connected, maximum supported is" + String(MAX_DEVICES));
      return;
    }

    // Some addresses represent non configurable devices (no MCU on it). Handle them as a special case.
    if (isFixedAddrDevice(addr)) {
      snprintf(buffer, 64, "0x%02X (cannot change)", addr);
      addRow(addr, fixedAddrToName(addr), "-", String(buffer));

      continue;  // Stop here, do not try to communicate with this device.
    }

    {
      uint8_t pinstrap = 0;           // Variable to store the pinstrap (device type)
      Wire1.beginTransmission(addr);  // Begin I2C transmission to the current address
      Wire1.write(0x00);              // Send a request to the device (assuming 0x00 is the register for device type)
      Wire1.endTransmission();        // End transmission

      delay(50);  // Delay to allow for the device to respond

      Wire1.requestFrom(addr, 1);  // Request 1 byte from the device at the current address
      if (Wire1.available()) {
        pinstrap = Wire1.read();  // Read the device type (pinstrap)
      } else {
        // If an error happens in the range 0x78 to 0x7F, ignore it.
        if (addr >= 0x78) continue;
        Serial.println("Failed to read device type at address 0x" + String(addr, HEX));
      }

      snprintf(buffer, 64, "0x%02X", pinstrap);
      auto hexPinstrap = String(buffer);

      snprintf(buffer, 64, "0x%02X", pinstrap / 2);  // Default address is half pinstrap.
      auto defaultAddr = String(buffer);
      if (addr != pinstrap / 2) defaultAddr += " *";  // Mark devices with modified address.

      addRow(addr, pinstrapToName(pinstrap), hexPinstrap, defaultAddr);
    }
  }

  // Print the results.
  for (int i = 0; i < numRows; i++) {
    char buffer[16];
    snprintf(buffer, 16, "0x%02X", rows[i].addr);

    Serial.print(fixedWidth(buffer, 8));
    Serial.print(fixedWidth(rows[i].modulinoType, 16));
    Serial.print(fixedWidth(rows[i].pinstrap, 16));
    Serial.println(fixedWidth(rows[i].defaultAddr, 12));
  }
}

void addRow(uint8_t address, String modulinoType, String pinstrap, String defaultAddr) {
  if (numRows >= MAX_DEVICES) return;

  rows[numRows].addr = address;
  rows[numRows].modulinoType = modulinoType;
  rows[numRows].pinstrap = pinstrap;
  rows[numRows].defaultAddr = defaultAddr;
  numRows++;  // Increment the row counter
}

bool findRow(uint8_t address) {
  for (int i = 0; i < numRows; i++) {
    if (rows[i].addr == address) return true;
  }
  return false;
}


// Function to add padding to the right to ensure each field has a fixed width
String fixedWidth(String str, int width) {
  for (int i = str.length(); i < width; i++) str += ' ';
  return str;
}

String pinstrapToName(uint8_t pinstrap) {
  switch (pinstrap) {
    case 0x04:
      return "Latch Relay";
    case 0x3C:
      return "Buzzer";
    case 0x58:
      return "Joystick";
    case 0x7C:
      return "Buttons";
    case 0x28:
      return "Opto Relay";
    case 0x76:
    case 0x74:
      return "Encoder";
    case 0x6C:
      return "Smartleds";
    case 0x70:
      return "Vibro";
  }
  return "UNKNOWN";
}

String fixedAddrToName(uint8_t address) {
  switch (address) {
    case 0x29:
      return "Distance";
    case 0x44:
      return "Thermo";
    case 0x6A:
    case 0x6B:
      return "Movement";
  }
  return "UNKNOWN";
}

bool isFixedAddrDevice(uint8_t addr) {
  // List of non-configurable devices, recognized by their fixed I2C address.
  const uint8_t fixedAddr[] = { 0x29, 0x44, 0x6A, 0x6B };

  for (int i = 0; i < sizeof(fixedAddr) / sizeof(fixedAddr[0]); i++) {
    if (addr == fixedAddr[i]) return true;
  }
  return false;
}
