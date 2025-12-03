/*
 * Modulino - Address Changer
 *
 * This example code is in the public domain. 
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include "Wire.h"

struct DetectedModulino {
  uint8_t addr;
  String modulinoType;
  String pinstrap;
  String defaultAddr;
};

#define MAX_DEVICES 16
DetectedModulino rows[MAX_DEVICES];
int numRows = 0;


void setup() {
  Wire1.begin();
  Serial.begin(115200);

  delay(600);
  discoverDevices();
}

bool waitingInput = false;
void loop() {
  if (numRows == 0) return;
  if (Serial.available() == 0 && waitingInput) return;

  if (Serial.available() > 0) {
    String hex1 = Serial.readStringUntil(' ');   // Read until space (or other delimiter)
    String hex2 = Serial.readStringUntil('\n');  // Read until newline
    Serial.println("> " + hex1 + " " + hex2);    // Print what the user inserted.

    int num1 = parseHex(hex1);  // Parse the first hex number
    int num2 = parseHex(hex2);  // Parse the second hex number
    if (num1 == -1 || num2 == -1) {
      Serial.println("Error: Incomplete or invalid input. Please enter two hexadecimal numbers");
      return;
    }

    bool success = updateI2cAddress(num1, num2);
    if (!success) return;  // If the update failed, skip discovery and messages, and wait for input again.

    discoverDevices();
    waitingInput = false;
  }

  Serial.println("Enter the current address, space, and new address (ex. \"0x20 0x30\" or \"20 2A\"):");
  Serial.println("  - Enter \"<addr> 0\" to reset the device at <addr> to its default address.");
  Serial.println("  - Enter \"0 0\" to reset all devices to the default address.");
  waitingInput = true;
}

// Updates the device at current address to new address. Supports broadcasting and setting default address (0).
// Returns true if the update was successful, false otherwise.
bool updateI2cAddress(int curAddress, int newAddress) {
  uint8_t data[40] = { 'C', 'F', newAddress * 2 };
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
  for (int addr = 8; addr < 128; addr++) {
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
