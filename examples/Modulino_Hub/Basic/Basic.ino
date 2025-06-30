#include <Modulino.h>

ModulinoHub hub;

ModulinoPixels light1(hub.port(1));
ModulinoPixels light2(hub.port(2));

void setup() {
  Modulino.begin();
  light1.begin();
  light2.begin();
}

void loop() {
  light1.set(0, BLUE, 50);
  light1.show();
  delay(100);
  light2.set(0, RED, 50);
  light2.show();
  delay(100);
}