/*
  Modulino LED Matrix Gallery Showcase

  This example demonstrates all the available icons and animations
  in the LEDMatrixGallery.h file.

  The sketch cycles through each icon for 2 seconds, and then plays
  each animation sequence.

  Circuit:
  - Modulino LED Matrix connected to the Modulino connector.

  This example code is in the public domain.
*/

#include "Modulino.h"
#include "Modulino_LED_Matrix.h"
#include "LEDMatrixGallery.h"

ModulinoLEDMatrix matrix;

void setup() {
  // Turn on the LED Matrix
  matrix.begin();
}

void loop() {
  
  // --- Icons ---
  
  matrix.setFrame(LEDMATRIX_BLUETOOTH);
  delay(2000);

  matrix.setFrame(LEDMATRIX_BOOTLOADER_ON);
  delay(2000);

  matrix.setFrame(LEDMATRIX_CHIP);
  delay(2000);

  matrix.setFrame(LEDMATRIX_CLOUD_WIFI);
  delay(2000);

  matrix.setFrame(LEDMATRIX_DANGER);
  delay(2000);

  matrix.setFrame(LEDMATRIX_EMOJI_BASIC);
  delay(2000);

  matrix.setFrame(LEDMATRIX_EMOJI_HAPPY);
  delay(2000);

  matrix.setFrame(LEDMATRIX_EMOJI_SAD);
  delay(2000);

  matrix.setFrame(LEDMATRIX_HEART_BIG);
  delay(2000);

  matrix.setFrame(LEDMATRIX_HEART_SMALL);
  delay(2000);

  matrix.setFrame(LEDMATRIX_LIKE);
  delay(2000);

  matrix.setFrame(LEDMATRIX_MUSIC_NOTE);
  delay(2000);

  matrix.setFrame(LEDMATRIX_RESISTOR);
  delay(2000);

  matrix.setFrame(LEDMATRIX_UNO);
  delay(2000);


  // --- Animations ---

  matrix.setSequence(LEDMATRIX_ANIMATION_STARTUP);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_TETRIS_INTRO);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_ATMEGA);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_LED_BLINK_HORIZONTAL);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_LED_BLINK_VERTICAL);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_ARROWS_COMPASS);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_AUDIO_WAVEFORM);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_BATTERY);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_BOUNCING_BALL);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_BUG);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_CHECK);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_CLOUD);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_DOWNLOAD);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_DVD);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_HEARTBEAT_LINE);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_HEARTBEAT);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_INFINITY_LOOP_LOADER);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_LOAD_CLOCK);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_LOAD);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_LOCK);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_NOTIFICATION);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_OPENSOURCE);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_SPINNING_COIN);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_TETRIS);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_WIFI_SEARCH);
  matrix.play();
  delay(1000);

  matrix.setSequence(LEDMATRIX_ANIMATION_HOURGLASS);
  matrix.play();
  delay(1000);

  // Clear the display before restarting
  matrix.clear();
  delay(1000);
}
