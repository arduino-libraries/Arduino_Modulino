#include <Arduino_Modulino.h>

ModulinoDisplay display;

void setup() {
    Serial.begin(115200);
    Modulino.begin();

    if (!display.begin()) {
        Serial.println("Display not found!");
        while (1);
    }

    display.beginDraw();
    display.background(0, 0, 0);
    display.stroke(255, 255, 255);
    display.textFont(Font_5x7);

    display.text("Hello Modulino!", 10, 10);
    display.noFill();
    display.circle(64, 40, 15);

    display.endDraw();
}

void loop() {
}