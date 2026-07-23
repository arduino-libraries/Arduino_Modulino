/**
 * This example shows how to use the Modulino LED Matrix library to display
 * basic graphics and animations on the Modulino LED Matrix display using the column-major order (vertical) mode. 
 * Frames in column-major order are more efficiently rendered on the LED matrix display, as they align with the way the display processes data.
 * 
 * Initial author: Sebastian Romero (s.romero@arduino.cc)
 */

#include <Arduino_Modulino.h>
#include "LEDMatrixGalleryVertical.h" // This header contains predefined animations for the LED matrix display.

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
  
  // Animations in column-major order require 
  // setting the display mode to MonochromaticVertical
  matrix.setMode(DisplayMode::MonochromaticVertical);
}

void loop() {
  // Play startup animation from gallery
  matrix.setSequence(LEDMATRIX_ANIMATION_STARTUP_VERTICAL);
  matrix.play();
  delay(500);

  // Show the UNO icon from the gallery
  matrix.setFrame(LEDMATRIX_UNO_VERTICAL);
  delay(1000);

  // Clear the display
  matrix.clear();
  delay(500);
}