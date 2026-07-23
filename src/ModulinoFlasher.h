#pragma once

// Copyright (c) 2025 Arduino SA
// SPDX-License-Identifier: MPL-2.0

#include "Modulino.h"
#include "ModulinoAddresses.h"

/**
 * @file ModulinoFlasher.h
 * @brief STM32 I2C bootloader protocol driver for writing Modulino firmware.
 *
 * Reference: STM32 I2C bootloader protocol
 * https://www.st.com/resource/en/application_note/an4221-i2c-protocol-used-in-the-stm32-bootloader-stmicroelectronics.pdf
 *
 * Usage:
 * @code
 * Module modulino;
 * ModulinoFlasher flasher(modulino);
 *
 * // After the target device is in bootloader mode and the reboot delay has elapsed:
 * flasher.flash(firmware_bin, firmware_bin_len);
 * @endcode
 */

// ─── SerialVerbose ────────────────────────────────────────────────────────────

/**
 * @brief Conditionally forwards print calls to Serial.
 *
 * Constructed with a @c verbose flag; all methods are no-ops when @c false.
 */
class SerialVerbose {
public:
  /**
   * @brief Constructs a SerialVerbose instance.
   * @param verbose If @c true, all print calls are forwarded to Serial.
   */
  SerialVerbose(bool verbose) : _verbose(verbose) {}

  /** @brief Prints a String if verbose mode is enabled. */
  void print(const String& s)           { if (_verbose) Serial.print(s); }
  /** @brief Prints an integer if verbose mode is enabled. */
  void print(int num, int base = DEC)   { if (_verbose) Serial.print(num, base); }
  /** @brief Prints a String followed by newline if verbose mode is enabled. */
  void println(const String& s)         { if (_verbose) Serial.println(s); }
  /** @brief Prints an integer followed by newline if verbose mode is enabled. */
  void println(int num, int base = DEC) { if (_verbose) Serial.println(num, base); }

private:
  bool _verbose;
};

// ─── ModulinoFlasher ──────────────────────────────────────────────────────────

/**
 * @brief Flashes Modulino firmware over I2C using the STM32 bootloader protocol.
 *
 * The target device must already be in bootloader mode at @ref BOOTLOADER_ADDR
 * before calling @ref flash(). Use @c ModulinoScanner::bootloaderReadyIn() to
 * ensure the required reboot delay has elapsed after a reset.
 */
class ModulinoFlasher {
public:
  /** @brief I2C address of the STM32 bootloader. */
  static constexpr uint8_t  BOOTLOADER_ADDR = MODULINO_BOOTLOADER_ADDR;
  /** @brief Size of a single flash write page in bytes. */
  static constexpr size_t   PAGE_SIZE       = 128;
  /** @brief Base flash memory address on supported STM32 targets. */
  static constexpr uint32_t FLASH_BASE_ADDR = 0x08000000;

  /**
   * @brief Progress callback invoked after groups of page writes.
   *
   * Signature: @c void callback(int currentPage, int totalPages)
   */
  using ProgressCallback = void(*)(int, int);

  /**
   * @brief Constructs a flasher bound to a Modulino bus instance.
   * @param module The @c Module whose I2C bus is used for all bootloader communication.
   */
  ModulinoFlasher(Module& module) : _module(module) {}

  /**
   * @brief Sends a software-reset command to a Modulino, causing it to reboot
   *        into the STM32 bootloader at @ref BOOTLOADER_ADDR.
   *
   * The Modulino firmware listens for the three-byte sequence @c {'D','I','E'}
   * and performs a software reset upon receipt. After calling this function,
   * wait at least @c ModulinoScanner::BOOTLOADER_REBOOT_MS milliseconds before
   * issuing @ref flash().
   *
   * @param address The current I2C address of the target Modulino.
   * @return @c true if the command was acknowledged, @c false if the device
   *         did not respond (e.g. already in bootloader mode or disconnected).
   */
  bool enterBootloader(uint8_t address) {
    uint8_t buf[128] = { 0 };
    buf[0] = 'D'; buf[1] = 'I'; buf[2] = 'E';
    
    // Modulino firmwares check that the received I2C payload matches their exact
    // struct size. We brute-force the length by padding with zeros until the device
    // stops responding at its app address (indicating it has reset into the bootloader).
    bool triggered = false;
    for (uint8_t s = 3; s <= 128; s++) {
      _module.getWire()->beginTransmission(address);
      _module.getWire()->write(buf, s);
      _module.getWire()->endTransmission();
      
      // Ping the device to see if it reset
      _module.getWire()->beginTransmission(address);
      if (_module.getWire()->endTransmission() != 0) {
        triggered = true;
        break;
      }
    }
    return triggered;
  }

  /**
   * @brief Erases flash memory and writes the given firmware image.
   *
   * Executes the full STM32 bootloader sequence: GET → GET_ID → MASS_ERASE →
   * WRITE_MEMORY (paged) → GO. Progress is reported to @p onProgress every
   * 4 pages and at completion.
   *
   * @param binary      Pointer to the firmware binary array.
   * @param length      Total size of the firmware in bytes.
   * @param verbose     If @c true, prints a detailed protocol trace to Serial.
   * @param onProgress  Optional callback invoked with @c (currentPage, totalPages).
   * @return @c true if the entire sequence succeeded, @c false on any failure.
   */
  bool flash(const uint8_t* binary, size_t length, bool verbose = false,
             ProgressCallback onProgress = nullptr) {
    SerialVerbose dbg(verbose);
    uint8_t resp_buf[255];

    dbg.println("GET_COMMAND");
    if (executeGet(resp_buf, 20, dbg) < 0) {
      Serial.println("Failed to get command. Ensure device is in bootloader mode.");
      return false;
    }
    for (int i = 0; i < 20; i++) dbg.println(resp_buf[i], HEX);

    dbg.println("GET_ID");
    int id_len = executeGetId(resp_buf, 3, dbg);
    for (int i = 0; i < id_len; i++) dbg.println(resp_buf[i], HEX);

    dbg.println("MASS_ERASE");
    Serial.println("Erasing flash...");
    if (!executeMassErase(dbg)) {
      Serial.println("Failed to completely erase flash.");
      return false;
    }

    Serial.println("Flashing...");
    int length_mod_page = ((length + PAGE_SIZE) / PAGE_SIZE) * PAGE_SIZE;
    int total_pages     = (length_mod_page / PAGE_SIZE) + 1;
    int current_page    = 0;

    for (int offset = length_mod_page; offset >= 0; offset -= PAGE_SIZE) {
      dbg.print("WRITE_PAGE OFFSET: ");
      dbg.println(offset, HEX);

      if (!executeWritePage(FLASH_BASE_ADDR + offset, &binary[offset], PAGE_SIZE, dbg)) {
        Serial.println("Write page operation failed at intermediate step!");
        return false;
      }

      delay(10);
      current_page++;

      if (current_page % 4 == 0 || current_page == total_pages) {
        int progress = (current_page * 100) / total_pages;
        Serial.print("Progress: ");
        Serial.print(progress);
        Serial.println("%");
        if (onProgress) onProgress(current_page, total_pages);
      }
    }

    Serial.println("Finishing up...");
    dbg.println("GO");
    if (!executeGo(FLASH_BASE_ADDR, dbg)) {
      Serial.println("Failed to perform the final GO sequence jump.");
      return false;
    }

    return true;
  }

private:
  Module& _module;

  // ─── Protocol constants ─────────────────────────────────────────────────────
  static constexpr uint8_t CMD_GET          = 0x00; ///< GET command opcode.
  static constexpr uint8_t CMD_GET_ID       = 0x02; ///< GET ID command opcode.
  static constexpr uint8_t CMD_GO           = 0x21; ///< GO command opcode.
  static constexpr uint8_t CMD_WRITE_MEMORY = 0x32; ///< WRITE MEMORY command opcode.
  static constexpr uint8_t CMD_MASS_ERASE   = 0x44; ///< MASS ERASE command opcode.
  static constexpr uint8_t ST_ACK           = 0x79; ///< ACK byte from the bootloader.
  static constexpr uint8_t ST_BUSY          = 0x76; ///< BUSY byte from the bootloader.

  // ─── Low-level helpers ──────────────────────────────────────────────────────

  /**
   * @brief Polls the bootloader until it sends an ACK (or a non-BUSY byte).
   *
   * @param dbg Debug logger.
   * @return @c true if ACK was received, @c false if a NACK or unexpected byte arrived.
   */
  bool waitForAck(SerialVerbose& dbg) {
    _module.getWire()->requestFrom(BOOTLOADER_ADDR, (uint8_t)1);
    auto c = _module.getWire()->read();
    while (c == ST_BUSY) {
      delay(10);
      _module.getWire()->requestFrom(BOOTLOADER_ADDR, (uint8_t)1);
      c = _module.getWire()->read();
    }
    if (c != ST_ACK) {
      dbg.print("error ack: ");
      dbg.println(c, HEX);
      return false;
    }
    return true;
  }

  /**
   * @brief Sends a command opcode followed by its bitwise inverse, then waits for ACK.
   *
   * @param opcode The command opcode byte.
   * @param dbg    Debug logger.
   * @return @c true if ACK was received.
   */
  bool sendOpcode(uint8_t opcode, SerialVerbose& dbg) {
    uint8_t cmd[2] = { opcode, (uint8_t)(0xFF ^ opcode) };
    _module.getWire()->beginTransmission(BOOTLOADER_ADDR);
    _module.getWire()->write(cmd, 2);
    _module.getWire()->endTransmission(true);
    return waitForAck(dbg);
  }

  /**
   * @brief Sends a data buffer with an XOR checksum, then waits for ACK.
   *
   * @param buffer            Data to send.
   * @param length            Number of bytes in @p buffer.
   * @param includeLengthByte If @c true, prepends @c (length - 1) before the data.
   * @param dbg               Debug logger.
   * @return @c true if ACK was received.
   */
  bool sendBufferWithChecksum(const uint8_t* buffer, size_t length,
                              bool includeLengthByte, SerialVerbose& dbg) {
    uint8_t checksum = 0;
    _module.getWire()->beginTransmission(BOOTLOADER_ADDR);
    if (includeLengthByte) {
      uint8_t lenByte = length - 1;
      _module.getWire()->write(lenByte);
      checksum ^= lenByte;
    }
    for (size_t i = 0; i < length; i++) {
      _module.getWire()->write(buffer[i]);
      checksum ^= buffer[i];
    }
    _module.getWire()->write(checksum);
    _module.getWire()->endTransmission(true);
    return waitForAck(dbg);
  }

  // ─── Bootloader commands ────────────────────────────────────────────────────

  /**
   * @brief Issues the GET command to retrieve the bootloader version and supported commands.
   *
   * @param output  Buffer to store the response.
   * @param max_len Maximum bytes to read into @p output.
   * @param dbg     Debug logger.
   * @return Number of bytes read, or @c -1 on failure.
   */
  int executeGet(uint8_t* output, int max_len, SerialVerbose& dbg) {
    if (!sendOpcode(CMD_GET, dbg)) return -1;
    _module.getWire()->requestFrom(BOOTLOADER_ADDR, (uint8_t)max_len);
    int howmany = _module.getWire()->read();
    for (int i = 0; i <= howmany; i++) output[i] = _module.getWire()->read();
    if (!waitForAck(dbg)) return -1;
    return howmany + 1;
  }

  /**
   * @brief Issues the GET ID command to retrieve the MCU product ID.
   *
   * @param output  Buffer to store the ID bytes.
   * @param max_len Maximum bytes to read.
   * @param dbg     Debug logger.
   * @return Number of ID bytes read, or @c -1 on failure.
   */
  int executeGetId(uint8_t* output, int max_len, SerialVerbose& dbg) {
    if (!sendOpcode(CMD_GET_ID, dbg)) return -1;
    _module.getWire()->requestFrom(BOOTLOADER_ADDR, (uint8_t)max_len);
    int howmany = _module.getWire()->read();
    for (int i = 0; i <= howmany; i++) output[i] = _module.getWire()->read();
    if (!waitForAck(dbg)) return -1;
    return howmany + 1;
  }

  /**
   * @brief Issues the global MASS ERASE command to wipe all flash pages.
   *
   * @param dbg Debug logger.
   * @return @c true if erase was acknowledged successfully.
   */
  bool executeMassErase(SerialVerbose& dbg) {
    if (!sendOpcode(CMD_MASS_ERASE, dbg)) return false;
    uint8_t erase_buf[2] = { 0xFF, 0xFF };
    return sendBufferWithChecksum(erase_buf, 2, false, dbg);
  }

  /**
   * @brief Issues the GO command to jump to the given flash address.
   *
   * @param address Target address (must be aligned to the MCU's vector table requirements).
   * @param dbg     Debug logger.
   * @return @c true if the bootloader acknowledged the jump.
   */
  bool executeGo(uint32_t address, SerialVerbose& dbg) {
    if (!sendOpcode(CMD_GO, dbg)) return false;
    uint8_t buf[4] = {
      (uint8_t)((address >> 24) & 0xFF),
      (uint8_t)((address >> 16) & 0xFF),
      (uint8_t)((address >> 8)  & 0xFF),
      (uint8_t)( address        & 0xFF)
    };
    return sendBufferWithChecksum(buf, 4, false, dbg);
  }

  /**
   * @brief Issues the WRITE MEMORY command to program one page of flash.
   *
   * @param address Destination flash address for this page.
   * @param buf_fw  Firmware data buffer (exactly @ref PAGE_SIZE bytes).
   * @param len_fw  Number of bytes to write.
   * @param dbg     Debug logger.
   * @return @c true if the page was written and acknowledged.
   */
  bool executeWritePage(uint32_t address, const uint8_t* buf_fw, size_t len_fw,
                        SerialVerbose& dbg) {
    if (!sendOpcode(CMD_WRITE_MEMORY, dbg)) return false;
    uint8_t addr_buf[4] = {
      (uint8_t)((address >> 24) & 0xFF),
      (uint8_t)((address >> 16) & 0xFF),
      (uint8_t)((address >> 8)  & 0xFF),
      (uint8_t)( address        & 0xFF)
    };
    if (!sendBufferWithChecksum(addr_buf, 4, false, dbg)) return false;
    return sendBufferWithChecksum(buf_fw, len_fw, true, dbg);
  }
};
