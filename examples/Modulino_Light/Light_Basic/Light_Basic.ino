#include "Modulino.h"

ModulinoLight light;

int lux;
int ir;

void setup() {
  Serial.begin(9600);
  Modulino.begin();
  light.begin();
}

void loop() {
  light.update();
  ModulinoColor color = light.getColor();
  String colorName = light.getColorApproximate();
  Serial.print("Color near to: ");
  Serial.print(colorName);

  int r = (0xFF000000 & color) >> 24;
  int g = (0x00FF0000 & color) >> 16;
  int b = (0x0000FF00 & color) >> 8;

  lux = light.getAL();
  ir = light.getIR();

  Serial.print("\tlight data: ");
  Serial.print("\tRed:\t");
  Serial.print(r);
  Serial.print("\tGreen:\t");
  Serial.print(g);
  Serial.print("\tBlue:\t");
  Serial.print(b);
  Serial.print("\tLux:\t");
  Serial.print(lux);
  Serial.print("\tIR\t");
  Serial.println(ir);
}
