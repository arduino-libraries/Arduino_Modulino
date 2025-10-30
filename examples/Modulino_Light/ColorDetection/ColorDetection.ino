#include "Modulino.h"

ModulinoLight light;

// before to run this sketch, change the encoders addresses in order to use more than one encoder
// and set the addresses in the constructor as shown below
ModulinoKnob knob_green;
ModulinoKnob knob_red(0x09);
ModulinoKnob knob_blue(0x0A);
ModulinoPixels leds;


int brightness = 100;
int red = 0;
int green = 0;
int blue = 0;


void setup() {
  Serial.begin(9600);
  Modulino.begin();
  knob_green.begin();
  knob_red.begin();
  knob_blue.begin();

  // set knobs initial positions to 0
  knob_red.set(0);
  knob_green.set(0);
  knob_blue.set(0);
  light.begin();
  leds.begin();
}

unsigned long start = millis();
void loop() {
  knobPressed();
  readColors();
  updatesColors();
}

void readColors() {
  if (red > 255) {
    red = 255;
    knob_red.set(red);
  } else if (red < 0) {
    red = 0;
    knob_red.set(red);
  }

  if (green > 255) {
    green = 255;
    knob_green.set(green);
  } else if (green < 0) {
    green = 0;
    knob_green.set(green);
  }

  if (blue > 255) {
    blue = 255;
    knob_blue.set(blue);
  } else if (blue < 0) {
    blue = 0;
    knob_red.set(blue);
  }
}

void knobPressed() {
  red = knob_red.get();
  green = knob_green.get();
  blue = knob_blue.get();
  bool red_click = knob_red.isPressed();
  bool green_click = knob_green.isPressed();
  bool blue_click = knob_blue.isPressed();

  if (red_click) {
    red = 255;
    knob_red.set(red);
  }

  if (green_click) {
    green = 255;
    knob_green.set(green);
  }

  if (blue_click) {
    blue = 255;
    knob_blue.set(blue);
  }

  if (green_click && blue_click && red_click ) {
    red = 0;
    knob_red.set(red);
    green = 0;
    knob_green.set(green);
    blue = 0;
    knob_blue.set(blue);
  } else if (red_click && green_click) {
    red = 255;
    knob_red.set(red);
    green = 255;
    knob_green.set(green);
    blue = 0;
    knob_blue.set(blue);
  } else if (red_click && blue_click) {
    red = 255;
    knob_red.set(red);
    green = 0;
    knob_green.set(green);
    blue = 255;
    knob_blue.set(blue);
  } else if (green_click && blue_click) {
    red = 0;
    knob_red.set(red);
    green = 255;
    knob_green.set(green);
    blue = 255;
    knob_blue.set(blue);
  }
}

void updatesColors() {
  ModulinoColor COLOR(red, green, blue);
  for (int l = 0; l < 8; l++) {
    leds.set(l, COLOR, brightness);
    leds.show();
  }
  light.update();

  if (millis() - start > 1000) {
    char buffer [50];
    int n, a = 3;
    n = sprintf (buffer, "%03d", red);

    Serial.print("Red:\t");
    Serial.print(buffer);
    n = sprintf (buffer, "%03d", green);
    Serial.print("\tGreen:\t");
    Serial.print(buffer);
    n = sprintf (buffer, "%03d", blue);
    Serial.print("\tBlue:\t");
    Serial.print(buffer);

    ModulinoColor color = light.getColor();
    String colorName = light.getColorApproximate();
    Serial.print("\tColor near to:\t");
    Serial.print(colorName);
    Serial.println();

    start = millis();
  }
}
