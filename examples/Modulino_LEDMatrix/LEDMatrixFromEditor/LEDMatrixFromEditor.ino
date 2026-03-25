/**
 * This example shows how to use the Modulino LED Matrix library to display
 * an animation on the Modulino LED Matrix display using frame data exported from the LED Matrix Editor tool.
 * See: https://ledmatrix-editor.arduino.cc/
 * 
 * Initial author: Sebastian Romero (s.romero@arduino.cc)
 */

#include "Modulino_LED_Matrix.h"
#include "animation.h"

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

  // Animations exported from the LED Matrix Editor tool 
  // are in row-major order.
  matrix.setMode(DisplayMode::MonochromaticHorizontal);
  matrix.setSequence(animation);
  matrix.play(true); // Loop the animation sequence
}

void loop() {}