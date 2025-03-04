#include "Modulino.h"

ModulinoVibro vibro;

void setup() {
  Modulino.begin();
  vibro.begin();
}

void loop() {
  vibro.on(1000);
  delay(1000);
  vibro.off();
  delay(1000);
}
