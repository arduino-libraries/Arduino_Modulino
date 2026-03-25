/**
 * This example shows how to use the Modulino LED Matrix library to display
 * grayscale graphics and animations on the Modulino LED Matrix display.
 * 
 * Initial author: Sebastian Romero (s.romero@arduino.cc)
 */

#include "Modulino_LED_Matrix.h"
#include "flames_animation.h"

/* Graphic in 4-bit grayscale */
constexpr uint8_t GRADIENT[] = { 	0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0x01, 0x23, 0x45, 0x67,
									0x89, 0xAB, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0x01, 0x23,
									0x45, 0x67, 0x89, 0xAB, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB};

ModulinoLEDMatrix matrix;

/**
 * Blinks the built-in LED to signal that the animation sequence is done.
 */
void blinkLED(){
  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  if (!matrix.begin()) {
    // If initialization fails, we enter an infinite loop and 
    // blink the built-in LED to indicate an error.
    while (true){
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Blink built-in LED to indicate error
      delay(500);
    }
  }

  // Set a callback to be called when the sequence is done
  matrix.setSequenceDoneCallback(blinkLED);

  // Set the display to Grayscale mode and show a gradient
  matrix.setMode(DisplayMode::Grayscale);
  matrix.setFrame(GRADIENT);
  delay(1000);

  // Play a grayscale animation in a loop
  matrix.setSequence(FLAMES);
  matrix.play(true);
}

void loop() {
}