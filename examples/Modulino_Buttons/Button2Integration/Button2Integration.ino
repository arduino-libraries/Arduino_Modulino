/*
 * Modulino Buttons - Button2 Integration
 * Integration with https://github.com/LennartHennigs/Button2
 *
 * This example code is in the public domain. 
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#include <Arduino_Modulino.h>
#include "Button2.h"

Button2 button;
ModulinoButtons modulino_buttons;

uint8_t button0StateHandler() {
  modulino_buttons.update();
  return modulino_buttons.isPressed(0) ? LOW : HIGH;  // fake a normal button -> LOW = pressed
}

void handler(Button2& btn) {
  switch (btn.getType()) {
    case single_click:
      break;
    case double_click:
      Serial.print("double ");
      break;
    case triple_click:
      Serial.print("triple ");
      break;
    case long_click:
      Serial.print("long");
      break;
  }
  Serial.print("click");
  Serial.print(" (");
  Serial.print(btn.getNumberOfClicks());
  Serial.println(")");
}

void setup() {

  Serial.begin(115200);

  Modulino.begin();
  modulino_buttons.begin();

  button.setDebounceTime(35);
  button.setButtonStateFunction(button0StateHandler);
  button.setClickHandler(handler);
  button.setDoubleClickHandler(handler);
  button.setTripleClickHandler(handler);
  button.setLongClickHandler(handler);
  button.begin(BTN_VIRTUAL_PIN);
}

void loop() {
  button.loop();
}