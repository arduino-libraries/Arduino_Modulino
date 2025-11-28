#include "Modulino.h"

ModulinoJoystick joystick;

void setup() {
  Serial.begin(9600);
  Modulino.begin();
  joystick.begin();
}

void loop() {
  joystick.update();

  if(joystick.isPressed()) {
    Serial.println("Pressed");
  }

  Serial.print("x,y: ");
  Serial.print(joystick.getX());
  Serial.print(", ");
  Serial.println(joystick.getY());
}
