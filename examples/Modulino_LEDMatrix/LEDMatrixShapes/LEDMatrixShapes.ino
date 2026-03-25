/*
* This example shows how to use the Modulino LED Matrix library to display
* basic shapes using the ArduinoGraphics library.
* The sketch cycles through displaying a point, a line, a rectangle, and a circle on the LED matrix.
* 
* Initial author: Sebastian Romero (s.romero@arduino.cc)
*/

#include "ArduinoGraphics.h"
#include "Modulino_LED_Matrix.h"

ModulinoLEDMatrix matrix;

void setup() {
  // Initialize the LED matrix
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
  // Draw a Point
  matrix.clear();
  matrix.beginDraw();
  matrix.stroke(0xFFFFFF); // Set color to white (ON)
  matrix.point(5, 4);      // Draw a point at x=5, y=4
  matrix.endDraw();
  delay(2000);

  // Draw a Line
  matrix.clear();
  matrix.beginDraw();
  matrix.stroke(0xFFFFFF);
  matrix.line(0, 0, 11, 7); // Draw a line from (0,0) to (11,7)
  matrix.endDraw();
  delay(2000);

  // Draw a Rectangle (outlined)
  matrix.clear();
  matrix.beginDraw();
  matrix.stroke(0xFFFFFF);
  matrix.noFill(); 
  // Rect parameters: x, y, width, height
  matrix.rect(2, 1, 8, 6); 
  matrix.endDraw();
  delay(2000);

  // Draw a Circle
  matrix.clear();
  matrix.beginDraw();
  matrix.stroke(0xFFFFFF);
  matrix.noFill();
  // Circle parameters: x, y, radius
  matrix.circle(6, 4, 3); 
  matrix.endDraw();
  delay(2000);
  
  // Draw a Filled Rectangle
  matrix.clear();
  matrix.beginDraw();
  matrix.stroke(0xFFFFFF);
  matrix.fill(0xFFFFFF); // Enable fill
  matrix.rect(3, 2, 6, 4);
  matrix.endDraw();
  delay(2000);
}
