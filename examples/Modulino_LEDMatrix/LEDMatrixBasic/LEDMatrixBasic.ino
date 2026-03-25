/**
 * This example shows how to use the Modulino LED Matrix library to display
 * basic graphics and animations on the Modulino LED Matrix display.
 * 
 * Initial author: Sebastian Romero (s.romero@arduino.cc)
 */

#include "Modulino_LED_Matrix.h"
#include "LEDMatrixGallery.h" // This header contains predefined animations for the LED matrix display.

ModulinoLEDMatrix matrix;

void setup() {
  if (!matrix.begin()) {
    // If initialization fails, we enter an infinite loop and 
    // blink the built-in LED to indicate an error.
    while (true){
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Blink built-in LED to indicate error
      delay(500);
    }
  }
}

void loop() {
  // Play startup animation from gallery
  matrix.setSequence(LEDMATRIX_ANIMATION_STARTUP);
  matrix.play();
  delay(500);

  // Show the UNO icon from the gallery
  matrix.setFrame(LEDMATRIX_UNO);
  delay(1000);

  // Clear the display
  matrix.clear();
  delay(500);
}