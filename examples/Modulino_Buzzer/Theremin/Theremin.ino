/*
 * Modulino Buzzer - Theremin
 * 
 * This example creates a theremin-like musical instrument using three Modulino modules:
 * - Distance sensor: Controls the pitch offset (closer = higher pitch)
 * - Knob (encoder): Selects the base musical note (12 chromatic notes)
 * - Buzzer: Plays the resulting tone
 * 
 * A theremin is an electronic musical instrument controlled without physical contact.
 * In this version, the distance sensor acts like proximity control, and the knob
 * selects which musical note to modify.
 * 
 * How it works:
 * 1. Turn the knob to select a base note (C, C#, D, D#, E, F, F#, G, G#, A, A#, or B)
 * 2. Move your hand closer/farther from the distance sensor to change the pitch
 * 3. The final pitch = base note frequency + distance measurement
 * 
 * Musical note frequencies (chromatic scale from C to B):
 * - C: 262 Hz, C#: 277 Hz, D: 294 Hz, D#: 311 Hz
 * - E: 330 Hz, F: 349 Hz, F#: 370 Hz, G: 392 Hz
 * - G#: 415 Hz, A: 440 Hz, A#: 466 Hz, B: 494 Hz
 * 
 * This example code is in the public domain.
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: CC0-1.0
 */

#include <Arduino_Modulino.h>

// Create objects for the three modules used
ModulinoBuzzer buzzer;
ModulinoDistance distance;
ModulinoKnob encoder;

void setup() {

  // Initialize Modulino I2C communication
  Modulino.begin();
  // Detect and connect to the encoder (knob) module
  encoder.begin();
  // Detect and connect to the distance sensor module
  distance.begin();
  // Detect and connect to the buzzer module
  buzzer.begin();
}

// Variable to store the pitch offset from the distance sensor (in Hz)
int pitch = 0;
// Variable to store the selected note index (0-11 for 12 chromatic notes)
int noteIndex = 0;

// Notes from C to B with sharps (#) - chromatic scale
// These are the base frequencies for one octave starting at middle C (C4)
int note[] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494};

void loop() {
  // Get distance measurement in millimeters and use it as pitch offset
  pitch = distance.get();
  // Get current encoder position to select which note to play
  noteIndex = encoder.get();

  // Use modulo to keep noteIndex within valid range (0-11)
  // This allows the encoder to wrap around through all 12 notes
  noteIndex = noteIndex % 12;
  // Ensure noteIndex doesn't go negative
  if (noteIndex < 0){
      noteIndex = 0;
  }

  // Play the selected note with pitch offset
  // Final frequency = base note frequency + distance offset
  // Duration is set to 1000ms to create continuous sound
  buzzer.tone(note[noteIndex] + pitch, 1000);
}