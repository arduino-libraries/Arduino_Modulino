/**
 * This example shows how to use the Modulino LED Matrix library to display
 * an animation on two LED matrix displays connected to the same I2C bus with different addresses.
 * play() is blocking, so we have to move through the frames manually using nextFrame() and renderFrame() 
 * to achieve simultaneous playback on both matrices.
 * 
 * Initial author: Sebastian Romero (s.romero@arduino.cc)
 */

#include "Modulino_LED_Matrix.h"

constexpr uint8_t animation[][16] = {
    { 0x00, 0x00, 0x00, 0x7c, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x7c, 0x82, 0x8a, 0x82, 0x82, 0x82, 0x7c, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x7c, 0x82, 0x8a, 0x82, 0x8a, 0x82, 0x7c, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x7c, 0x82, 0x8a, 0xa2, 0x8a, 0x82, 0x7c, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00 },
    { 0x00, 0x00, 0x00, 0x7c, 0x82, 0xaa, 0xa2, 0xaa, 0x82, 0x7c, 0x00, 0x00, 0xe8, 0x03, 0x00, 0x00 },
};

// First matrix uses the default I2C address (0x39).
ModulinoLEDMatrix matrix1;

// Example of using a different I2C address for the second matrix. 
// You can use the AddressChanger.ino example to change the address of the second matrix.
// Change this address to the actual address of your second matrix.
ModulinoLEDMatrix matrix2(0x40);

void setup() {
  if (!matrix1.begin()) {
    // If initialization fails, we enter an infinite loop and 
    // blink the built-in LED to indicate an error.
    while (true){
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Blink built-in LED to indicate error
      delay(500);
    }
  }
  
  if (!matrix2.begin()) {
    // If initialization fails, we enter an infinite loop and 
    // blink the built-in LED to indicate an error.
    while (true){
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN)); // Blink built-in LED to indicate error
      delay(500);
    }
  }

  // Clear the displays before starting the animation in case there is any residual data on the matrices.
  matrix1.clear();
  matrix2.clear();

  // Load the animation sequence into both matrices. 
  // The setSequence method can accept a 2D array of bytes, where each row represents a frame in the animation. 
  // The library will handle the timing and rendering of the frames based on the data provided.
  matrix1.setSequence(animation);
  matrix2.setSequence(animation);
}

void loop() {
    auto frameCount = matrix1.getFrameCount();
    
    /**
     * This loop demonstrates how to play the animation forward on both matrices simultaneously.
     * It uses the nextFrame() method to advance through the frames in the sequence, and retrieves the duration for each frame to ensure proper timing.
     */
    for (int i = 0; i < frameCount; i++) {
        matrix1.nextFrame();
        matrix2.nextFrame();
        auto duration = matrix1.getCurrentDuration();
        delay(duration);
    }

    /**
     * This loop demonstrates how to play the animation backward on both matrices simultaneously.
     * It uses the renderFrame() method to directly render a specific frame in the sequence, allowing for reverse playback.
     */
    for (int i = frameCount - 1; i >= 0; i--) {
        matrix1.renderFrame(i);
        matrix2.renderFrame(i);
        auto duration = matrix1.getCurrentDuration();
        delay(duration);
    }
}