/**
 * This example shows how to use the Modulino LED Matrix library to display
 * scrolling text on the Modulino LED Matrix display using the ArduinoGraphics library.
 * 
 * Initial author: Sebastian Romero (s.romero@arduino.cc)
 */

#include "ArduinoGraphics.h"
#include "Modulino_LED_Matrix.h"

ModulinoLEDMatrix matrix;

/**
 * Displays scrolling text on the LED matrix display.
 * The text is scrolled from right to left with a specified speed and font.
 * @param text The text to be displayed on the LED matrix.
 * @param font The font to be used for the text display (default is Font_4x6).
 * @param speed The speed of the scrolling text in milliseconds (default is 100 ms).
 */
void scrollText(const String& text, const Font& font = Font_4x6, unsigned long speed = 100) {
  matrix.textScrollSpeed(speed);
  matrix.textFont(font);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText(SCROLL_LEFT);
}

void setup() {
  matrix.begin();
}

void loop() {
  scrollText("  Modulino  ");
  scrollText("    LED Matrix    ", Font_5x7, 50);
}