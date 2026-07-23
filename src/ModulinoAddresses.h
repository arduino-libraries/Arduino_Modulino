#pragma once

// Copyright (c) 2025 Arduino SA
// SPDX-License-Identifier: MPL-2.0

#include <stdint.h>
#include <Arduino.h>

/**
 * @file ModulinoAddresses.h
 * @brief Central registry of Modulino I2C addresses, device names and firmware types.
 * All devices are stored in a single @ref MODULINO_MAP table.
 * A non-zero @c pinstrap field marks a configurable device whose default I2C
 * address is @c pinstrap / 2. A zero @c pinstrap marks a fixed-address device
 * whose hard-coded address is stored in @c addr.
 * To register a new device, append one entry to @ref MODULINO_MAP.
 */

/**
 * @brief Firmware image required to program a Modulino device.
 */
enum ModulinoFirmware : uint8_t {
  FIRMWARE_GENERIC,    ///< Standard firmware (Buzzer, Buttons, Knob, Pixels, ...).
  FIRMWARE_MOTORS,     ///< Motors module firmware.
  FIRMWARE_LED_MATRIX, ///< LED Matrix module firmware.
};

/**
 * @brief Describes one entry in the unified Modulino device registry.
 * - **Configurable device** (@c pinstrap != 0): responds to I2C address
 *   @c pinstrap / 2 by default and can be re-addressed. The @c addr field
 *   holds that default address for convenience.
 * - **Fixed-address device** (@c pinstrap == 0): uses @c addr as its
 *   permanent, non-configurable I2C address.
 */
struct ModulinoEntry {
  uint8_t          addr;     ///< Default I2C address (pinstrap/2) for configurable devices, or the fixed address.
  uint8_t          pinstrap; ///< Pinstrap byte reported by the device. @c 0 for fixed-address devices.
  const char*      name;     ///< Human-readable device name.
  ModulinoFirmware firmware; ///< Firmware image required to program this device.
};

/**
 * @brief Unified lookup table of all known Modulino devices.
 * Entries with @c pinstrap != 0 are configurable; entries with @c pinstrap == 0
 * are fixed-address. Within each group, entries are sorted by address.
 */
static const ModulinoEntry MODULINO_MAP[] = {
  // --- Configurable devices (pinstrap != 0, default addr = pinstrap / 2) ---
  { 0x02, 0x04, "Latch Relay", FIRMWARE_GENERIC    },
  { 0x14, 0x28, "Opto Relay",  FIRMWARE_GENERIC    },
  { 0x1E, 0x3C, "Buzzer",      FIRMWARE_GENERIC    },
  { 0x24, 0x48, "Motors",      FIRMWARE_MOTORS     },
  { 0x2C, 0x58, "Joystick",    FIRMWARE_GENERIC    },
  { 0x36, 0x6C, "Pixels",      FIRMWARE_GENERIC    },
  { 0x38, 0x70, "Vibro",       FIRMWARE_GENERIC    },
  { 0x39, 0x72, "LED Matrix",  FIRMWARE_LED_MATRIX },
  { 0x3A, 0x74, "Knob",        FIRMWARE_GENERIC    },
  { 0x3B, 0x76, "Knob",        FIRMWARE_GENERIC    },
  { 0x3E, 0x7C, "Buttons",     FIRMWARE_GENERIC    },
  // --- Fixed-address devices (pinstrap == 0) ---
  { 0x29, 0x00, "Distance",    FIRMWARE_GENERIC    },
  { 0x44, 0x00, "Thermo",      FIRMWARE_GENERIC    },
  { 0x64, 0x00, "Bootloader",  FIRMWARE_GENERIC    },
  { 0x6A, 0x00, "Movement",    FIRMWARE_GENERIC    },
  { 0x6B, 0x00, "Movement",    FIRMWARE_GENERIC    },
};

/** @brief Number of entries in @ref MODULINO_MAP. */
static constexpr size_t MODULINO_MAP_SIZE =
    sizeof(MODULINO_MAP) / sizeof(MODULINO_MAP[0]);

// --- Well-known address constants ---

/** @brief I2C address used by the STM32 bootloader on all configurable Modulino modules. */
static constexpr uint8_t MODULINO_BOOTLOADER_ADDR = 0x64;

// --- Lookup helpers ---

/**
 * @brief Resolves a device name from a pinstrap byte.
 * @param pinstrap The pinstrap value read from the device.
 * @return The device name, or @c "UNKNOWN" if not in @ref MODULINO_MAP.
 */
inline String modulinoPinstrapToName(uint8_t pinstrap) {
  for (size_t i = 0; i < MODULINO_MAP_SIZE; i++) {
    if (MODULINO_MAP[i].pinstrap == pinstrap) return String(MODULINO_MAP[i].name);
  }
  return "UNKNOWN";
}

/**
 * @brief Resolves a device name from a fixed I2C address.
 * Only searches entries whose @c pinstrap is @c 0.
 * @param address The fixed I2C address of the device.
 * @return The device name, or @c "UNKNOWN" if not in @ref MODULINO_MAP.
 */
inline String modulinoFixedAddrToName(uint8_t address) {
  for (size_t i = 0; i < MODULINO_MAP_SIZE; i++) {
    if (MODULINO_MAP[i].pinstrap == 0 && MODULINO_MAP[i].addr == address) {
      return String(MODULINO_MAP[i].name);
    }
  }
  return "UNKNOWN";
}

/**
 * @brief Returns @c true if @p addr belongs to a fixed-address device.
 * A device is fixed-address when it has a @c pinstrap of @c 0 in @ref MODULINO_MAP.
 * @param addr The I2C address to test.
 */
inline bool modulinoIsFixedAddrDevice(uint8_t addr) {
  for (size_t i = 0; i < MODULINO_MAP_SIZE; i++) {
    if (MODULINO_MAP[i].pinstrap == 0 && MODULINO_MAP[i].addr == addr) return true;
  }
  return false;
}

/**
 * @brief Returns the firmware image required for the device with the given pinstrap.
 * @param pinstrap The device's pinstrap byte.
 * @return The @ref ModulinoFirmware for that device, or @c FIRMWARE_GENERIC
 *         if the pinstrap is not found in @ref MODULINO_MAP.
 */
inline ModulinoFirmware modulinoFirmwareForPinstrap(uint8_t pinstrap) {
  for (size_t i = 0; i < MODULINO_MAP_SIZE; i++) {
    if (MODULINO_MAP[i].pinstrap == pinstrap) return MODULINO_MAP[i].firmware;
  }
  return FIRMWARE_GENERIC;
}
