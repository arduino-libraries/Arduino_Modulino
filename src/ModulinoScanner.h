#pragma once

// Copyright (c) 2025 Arduino SA
// SPDX-License-Identifier: MPL-2.0

#include "Modulino.h"
#include "ModulinoAddresses.h"

/**
 * @file ModulinoScanner.h
 * @brief I2C bus scanner for discovering connected Modulino devices.
 *
 * Address and name lookups are delegated to @ref ModulinoAddresses.h so that
 * a single file is the authoritative source for all device metadata.
 *
 * Usage:
 * @code
 * Module modulino;
 * ModulinoScanner scanner;
 *
 * Modulino.begin();
 * scanner.discover(modulino);
 * scanner.printDevices();
 * @endcode
 */

/**
 * @brief Holds the detected properties of a single Modulino device.
 */
struct DetectedModulino {
  uint8_t addr;          ///< I2C address at which the device was found.
  String modulinoType;   ///< Human-readable device type name.
  bool isFixed;          ///< @c true if the device uses a fixed, non-configurable I2C address.
  uint8_t pinstrapValue; ///< Raw pinstrap byte as read from the device, or @c 0 for fixed-address devices.
};

/**
 * @brief Scans an I2C bus and maintains a list of discovered Modulino devices.
 */
class ModulinoScanner {
public:
  /** @brief Maximum number of devices tracked in a single scan. */
  static constexpr int MAX_DEVICES = 16;

  /**
   * @brief Time in milliseconds for the STM32 bootloader to reboot after
   *        being contacted during a scan or after receiving a reset command.
   */
  static constexpr unsigned long BOOTLOADER_REBOOT_MS = 6500;

  DetectedModulino devices[MAX_DEVICES]; ///< Array of discovered devices.
  int numDevices = 0;                    ///< Number of valid entries in @ref devices.

  /**
   * @brief Timestamp (@c millis()) of the most recent bootloader reset.
   * Updated automatically by @ref discover() when a device at
   * @ref MODULINO_BOOTLOADER_ADDR is found, and manually via
   * @ref notifyBootloaderHasReset() after sending a software-reset command.
   */
  unsigned long lastBootloaderContactMs = 0;

  /** @brief Default constructor. Call @ref begin() before @ref discover(). */
  ModulinoScanner() : _wire(nullptr) {}

  /**
   * @brief Constructs a scanner bound to the given I2C bus.
   * @param wire The I2C peripheral to use for all scan operations.
   */
  explicit ModulinoScanner(HardwareI2C& wire) : _wire(&wire) {}

  /**
   * @brief Binds the scanner to an I2C bus.
   *
   * Call this once in @c setup() after @c Modulino.begin(), or use the
   * single-argument constructor instead.
   *
   * @param wire The I2C peripheral to use for all scan operations.
   */
  void begin(HardwareI2C& wire) { _wire = &wire; }

  /**
   * @brief Pings the bootloader address to check if a device is currently in bootloader mode.
   *
   * If the bootloader acknowledges the ping, @ref notifyBootloaderHasReset() is called 
   * automatically because the probe causes the bootloader to reset.
   *
   * @return @c true if a device is in bootloader mode, @c false otherwise.
   */
  bool deviceInBootloaderMode() {
    if (!_wire) return false;
    _wire->beginTransmission(MODULINO_BOOTLOADER_ADDR);
    if (_wire->endTransmission() == 0) {
      notifyBootloaderHasReset();
      return true;
    }
    return false;
  }

  /**
   * @brief Scans the I2C bus and populates the internal device list.
   *
   * Uses the @c HardwareI2C bus provided at construction or via @ref begin().
   * For each responding address, checks @ref modulinoIsFixedAddrDevice() to
   * determine device type. Configurable devices are queried for their pinstrap
   * byte. If @ref MODULINO_BOOTLOADER_ADDR responds, @ref notifyBootloaderHasReset()
   * is called automatically because the probe causes the bootloader to reset.
   */
  void discover() {
    if (!_wire) return;
    numDevices = 0;

    for (int addr = 1; addr < 128; addr++) {
      _wire->beginTransmission((uint8_t)addr);
      if (_wire->endTransmission() != 0) continue;
      if (numDevices >= MAX_DEVICES) return;

      if (modulinoIsFixedAddrDevice((uint8_t)addr)) {
        if ((uint8_t)addr == MODULINO_BOOTLOADER_ADDR) notifyBootloaderHasReset();
        addDevice((uint8_t)addr, modulinoFixedAddrToName((uint8_t)addr), true, 0);
        continue;
      }

      uint8_t pinstrap = 0;
      _wire->requestFrom((uint8_t)addr, (uint8_t)1);
      if (_wire->available()) {
        pinstrap = _wire->read();
      } else {
        // Ignore reserved I2C addresses (0x78-0x7F) if they return no data
        if (addr >= 0x78) continue;
      }

      addDevice((uint8_t)addr, modulinoPinstrapToName(pinstrap), false, pinstrap);
    }
  }

  /**
   * @brief Prints the discovered device list to the Serial monitor.
   *
   * Outputs a formatted table with columns: ADDR, MODULINO, PINSTRAP, DEFAULT ADDR.
   * When @p showIndex is @c true, each row is prefixed with a 1-based index number
   * so the user can select a device by typing its number.
   *
   * @param showIndex If @c true, prepend a @c [n] index to every row.
   */
  void printDevices(bool showIndex = false) {
    if (showIndex) Serial.print("     ");
    Serial.print(padToWidth("ADDR", 8));
    Serial.print(padToWidth("MODULINO", 16));
    Serial.print(padToWidth("PINSTRAP", 16));
    Serial.println("DEFAULT ADDR");
    for (int i = 0; i < numDevices; i++) {
      if (showIndex) {
        char idx[6];
        snprintf(idx, sizeof(idx), "[%2d] ", i + 1);
        Serial.print(idx);
      }
      char buffer[16];
      snprintf(buffer, 16, "0x%02X", devices[i].addr);
      Serial.print(padToWidth(buffer, 8));
      Serial.print(padToWidth(devices[i].modulinoType, 16));
      if (devices[i].isFixed) {
        Serial.print(padToWidth("-", 16));
        Serial.println(padToWidth(buffer, 12));  // addr == default for fixed devices
      } else {
        snprintf(buffer, 16, "0x%02X", devices[i].pinstrapValue);
        Serial.print(padToWidth(buffer, 16));
        snprintf(buffer, 16, "0x%02X", devices[i].pinstrapValue / 2);
        String defAddr = buffer;
        if (devices[i].addr != devices[i].pinstrapValue / 2) defAddr += " *";
        Serial.println(padToWidth(defAddr, 12));
      }
    }
  }

  /**
   * @brief Finds a device by address in the last scan result.
   *
   * Returns a pointer into the internal @ref devices array, giving the caller
   * direct access to type, pinstrap, and @ref DetectedModulino::isFixed without
   * a separate @ref modulinoIsFixedAddrDevice() call.
   *
   * @param addr The I2C address to search for.
   * @return Pointer to the matching @ref DetectedModulino, or @c nullptr if not found.
   */
  const DetectedModulino* getDevice(uint8_t addr) {
    for (int i = 0; i < numDevices; i++) {
      if (devices[i].addr == addr) return &devices[i];
    }
    return nullptr;
  }

  /**
   * @brief Notifies the scanner that the STM32 bootloader has just been reset.
   * Records the current time so @ref bootloaderReadyIn() can compute the
   * remaining reboot delay. Call this after @ref ModulinoFlasher::enterBootloader()
   * returns @c true, or whenever you know the bootloader has been triggered.
   */
  void notifyBootloaderHasReset() {
    lastBootloaderContactMs = millis();
  }

  /**
   * @brief Returns milliseconds remaining until the bootloader is ready for commands.
   * After the STM32 bootloader is contacted (either by a scan or by a reset command),
   * it takes @ref BOOTLOADER_REBOOT_MS to re-enumerate. This method returns how much
   * of that time is still outstanding. Feed the result into @c delay() before calling
   * @ref ModulinoFlasher::flash().
   * @return Remaining wait in milliseconds, or @c 0 if the device should be ready.
   */
  unsigned long bootloaderReadyIn() const {
    if (lastBootloaderContactMs == 0) return 0;
    unsigned long elapsed = millis() - lastBootloaderContactMs;
    if (elapsed >= BOOTLOADER_REBOOT_MS) return 0;
    return BOOTLOADER_REBOOT_MS - elapsed;
  }

private:
  HardwareI2C* _wire; ///< I2C bus used for scanning. Must be set via constructor or @ref begin().

  /**
   * @brief Appends a device entry to the internal list.
   * Does nothing if the list is already at capacity (@ref MAX_DEVICES).
   * @param address       The I2C address of the device.
   * @param modulinoType  Human-readable device type name.
   * @param isFixed       Whether the device has a fixed, non-configurable address.
   * @param pinstrapValue Raw pinstrap byte, or @c 0 for fixed-address devices.
   */
  void addDevice(uint8_t address, String modulinoType, bool isFixed, uint8_t pinstrapValue) {
    if (numDevices >= MAX_DEVICES) return;
    devices[numDevices].addr = address;
    devices[numDevices].modulinoType = modulinoType;
    devices[numDevices].isFixed = isFixed;
    devices[numDevices].pinstrapValue = pinstrapValue;
    numDevices++;
  }

  /**
   * @brief Pads @p str with trailing spaces to a minimum display width.
   * @param str   The string to pad.
   * @param width Desired minimum character width.
   * @return The padded @c String.
   */
  String padToWidth(String str, int width) {
    for (int i = str.length(); i < width; i++) str += ' ';
    return str;
  }
};
