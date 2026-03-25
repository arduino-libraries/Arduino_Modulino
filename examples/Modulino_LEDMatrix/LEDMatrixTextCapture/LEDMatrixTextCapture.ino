/**
 * This example shows how to use the Modulino LED Matrix library to capture
 * text animations into a buffer and then play them back on the Modulino LED Matrix display.
 * This is useful for creating complex text animations that can be stored and played without needing to run the text rendering logic in real-time.
 * 
 * Initial author: Sebastian Romero (s.romero@arduino.cc)
 */

#include "ArduinoGraphics.h"
#include "Modulino_LED_Matrix.h"

ModulinoLEDMatrix matrix;

// Define buffers to hold the animations.
// 128 frames should be enough for most text animations.
// Each frame needs space for pixels + duration.
// For Modulino 12x8, that's 12 bytes pixels + 4 bytes duration = 16 bytes.
constexpr size_t FRAMES = 128;

// Frame buffers for the animations. This is where the captured frames will be stored.
uint8_t animation1Buffer[FRAMES][MONOCHROMATIC_ANIMATION_FRAME_SIZE]; 
uint8_t animation2Buffer[FRAMES][MONOCHROMATIC_ANIMATION_FRAME_SIZE];

// Variables to hold the actual used size of the buffers after capturing the animations.
// This is important because the captured animation might not use all the allocated space.
uint32_t animation1UsedBytes = 0;
uint32_t animation2UsedBytes = 0;

void prepareAnimations(){
  // Setup the text properties
  matrix.textScrollSpeed(50); // 50ms per frame
  matrix.textFont(Font_5x7);

  // Prepare animation 1
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print("  Hello  ");
  // Note: We do NOT call matrix.endText() here.

  // Capture the animation into the buffer
  // This runs the scrolling text simulation internally and fills the animation buffer.
  // It happens much faster than real-time because there's no delay or I2C communication.
  // We pass the buffer and the variable to store the used size.
  matrix.endTextAnimation(SCROLL_LEFT, animation1Buffer, animation1UsedBytes);
  
  // Prepare a second animation with different text
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.print(" Modulino LED Matrix ");
  matrix.endTextAnimation(SCROLL_LEFT, animation2Buffer, animation2UsedBytes);
  
  // Print information about the captured animations
  Serial.print("Bytes used for animation 1: ");
  Serial.println(animation1UsedBytes);
  Serial.print("Remaining bytes for animation 1: ");
  Serial.println(sizeof(animation1Buffer) - animation1UsedBytes);
  Serial.print("Frames captured for animation 1: ");
  Serial.println(animation1UsedBytes / MONOCHROMATIC_ANIMATION_FRAME_SIZE);
  
  Serial.print("Bytes used for animation 2: ");
  Serial.println(animation2UsedBytes);
  Serial.print("Remaining bytes for animation 2: ");
  Serial.println(sizeof(animation2Buffer) - animation2UsedBytes);
  Serial.print("Frames captured for animation 2: ");
  Serial.println(animation2UsedBytes / MONOCHROMATIC_ANIMATION_FRAME_SIZE);
}

void setup() {
  Serial.begin(115200);
  matrix.begin();
  prepareAnimations();
}

void loop() {
  // Load the first captured sequence into the matrix engine
  // This tells the matrix to play the frames stored in 'animation1Buffer' with the durations captured in each frame.
  // We pass the buffer and the exact number of used bytes.
  matrix.setSequence(animation1Buffer, animation1UsedBytes);
  
  // Start playing (blocks until the sequence is done).
  // The matrix will now cycle through the captured frames using the durations stored in them.
  matrix.play();

  // After the first animation is done, we can load and play the second one.
  matrix.setSequence(animation2Buffer, animation2UsedBytes);
  matrix.play();
}
