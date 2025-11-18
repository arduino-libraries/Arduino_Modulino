#include "Modulino.h"

ModulinoVibro vibro;

void setup() {
  Modulino.begin();
  vibro.begin();
}

void loop() {
  // Cycle through vibration intensities from GENTLE to MAXIMUM
  // Each intensity increases by 5, lasting 1 seconds each
  for (int intensity = GENTLE; intensity <= MAXIMUM; intensity += 5) {
    Serial.println(intensity);
    vibro.on(1000, true, intensity);
    vibro.off();
    delay(100);
  }

  // Two vibration of one second separated by a long pause
  // using the blocking on call
  delay(1000);
  vibro.on(1000);
  delay(5000);
  vibro.on(1000);
  delay(1000);
}
