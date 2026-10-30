/*
 * Modulino Load Cell - Basic
 *
 * Reads a load cell on channel A and prints the weight in grams.
 *
 * Wiring of a 4-wire load cell to J5 (channel A), by the labels on the board:
 *   3V   red    excitation +
 *   A-   white  signal -
 *   A+   green  signal +
 *   GND  black  excitation -   (white mark next to the terminal)
 * Colours vary between manufacturers: check the cell's datasheet. If the weight
 * goes down when you load the cell, swap the A+ and A- wires.
 * For a cell on J6 (channel B, terminals B- and B+), call
 * scale.setMode(ModulinoLoadCell::Mode::B) after begin() and pass
 * ModulinoLoadCell::CHANNEL_B to the other calls.
 *
 * Set CELL_MV_PER_V and CELL_CAPACITY_G from the cell's datasheet. The reading is
 * then as accurate as the cell's rated output (often 10 to 20 %). For an accurate
 * scale, calibrate once with the LoadCell_Calibration example: the calibration is
 * stored in the module and used from then on.
 *
 * This example code is in the public domain.
 * Copyright (c) 2026 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

ModulinoLoadCell scale;

const float CELL_MV_PER_V = 1.0;       // rated output, mV/V
const uint32_t CELL_CAPACITY_G = 5000;  // rated capacity, grams

void setup() {
  Serial.begin(9600);
  Modulino.begin();
  if (!scale.begin()) {
    Serial.println("Modulino Load Cell not found, or its firmware needs an update (FirmwareUpdater example)");
    while (true) {}
  }

  // Use the datasheet values unless the module holds a calibration
  if (!scale.hasStoredCalibration()) {
    scale.setLoadCell(CELL_MV_PER_V, CELL_CAPACITY_G);
  }
  scale.setFilter(300);  // smooth the reading over about 0.3 s

  Serial.println("Taring, keep the cell unloaded...");
  scale.tare();
}

void loop() {
  if (scale.available()) {
    Serial.print(scale.getWeight(), 1);
    Serial.print(" g");
    if (scale.isOverCapacity()) {
      Serial.print("  over capacity!");
    }
    Serial.println();
  }
}
