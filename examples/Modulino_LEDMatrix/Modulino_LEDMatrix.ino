#include "Modulino_LED_Matrix.h"

ModulinoLEDMatrix matrix;

void showEmoji(){
    matrix.loadFrame(LEDMATRIX_EMOJI_HAPPY);
    delay(1000);
    matrix.loadFrame(LEDMATRIX_EMOJI_SAD);
    delay(1000);
}

void setup() {
  Serial.begin(115200);
  
  if (!matrix.begin()) {
    Serial.println("Failed to start Modulino LED Matrix!");
    while (1);
  }

  for (int i = 0; i < 3; i++) {
      showEmoji();
  }

  matrix.loadSequence(LEDMATRIX_ANIMATION_STARTUP);
  matrix.play();
}

void loop() {
}