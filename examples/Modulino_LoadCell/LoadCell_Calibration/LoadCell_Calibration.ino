/*
 * Modulino Load Cell - Calibration
 *
 * Calibrates the cell on channel A (J5) or B (J6) with a known weight and stores
 * the calibration in the module, so every sketch reads correct grams from then on.
 * Run it once per cell: the calibration of the other channel is kept.
 *
 * Open the Serial Monitor (9600 baud, "Newline") and follow the prompts:
 *   1. remove everything from the cell and send A or B: the module tares that channel
 *   2. place a known weight (at least 10 % of the cell's capacity) and send its
 *      value in grams, for example 500
 *   3. the module computes the scale, stores it, and the sketch prints the weight
 * Send "e" at any time to erase the stored calibration.
 *
 * This example code is in the public domain.
 * Copyright (c) 2026 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>

ModulinoLoadCell scale;
uint8_t channel = ModulinoLoadCell::CHANNEL_A;

const uint32_t CELL_CAPACITY_G = 5000;  // rated capacity, grams (for the over-capacity warning)

String readLine() {
  while (!Serial.available()) {}
  String line = Serial.readStringUntil('\n');
  line.trim();
  return line;
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  Modulino.begin();
  if (!scale.begin()) {
    Serial.println("Modulino Load Cell not found, or its firmware needs an update (FirmwareUpdater example)");
    while (true) {}
  }

  Serial.println("Remove everything from the load cell, then send A (cell on J5) or B (cell on J6).");
  String answer = readLine();
  if (answer == "e") {
    scale.eraseCalibration();
    Serial.println("Stored calibration erased.");
    while (true) {}
  }
  if (answer == "B" || answer == "b") {
    channel = ModulinoLoadCell::CHANNEL_B;
  }
  // tare() and calibrate() average samples of the channel: sample only that one
  scale.setMode(channel == ModulinoLoadCell::CHANNEL_A ? ModulinoLoadCell::Mode::A : ModulinoLoadCell::Mode::B);
  if (!scale.tare(channel)) {
    Serial.print("Tare failed, error ");
    Serial.println((int)scale.getLastResult());
    while (true) {}
  }

  Serial.println("Place a known weight on the cell and send its value in grams.");
  float known = readLine().toFloat();
  if (known <= 0 || !scale.calibrate(known, channel)) {
    Serial.print("Calibration failed, error ");
    Serial.println((int)scale.getLastResult());
    while (true) {}
  }
  scale.setCapacity(CELL_CAPACITY_G, channel);
  scale.setFilter(300, channel);

  if (scale.saveCalibration()) {
    Serial.println("Calibration stored in the module.");
  }
  scale.readSettings(channel);
  Serial.print("Scale: ");
  Serial.print(scale.getScale(channel), 2);
  Serial.println(" counts per gram");
}

void loop() {
  if (Serial.available() && readLine() == "e") {
    scale.eraseCalibration();
    Serial.println("Stored calibration erased.");
  }
  if (scale.available(channel)) {
    Serial.print(scale.getWeight(channel), 1);
    Serial.println(" g");
  }
}
