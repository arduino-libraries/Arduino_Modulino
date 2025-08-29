#include <Modulino.h>

//Maximum and minimum encoder values
#define KNOBMAX 7
#define KNOBMIN 0

//Modulino boards
ModulinoKnob knob;
ModulinoPixels leds;
ModulinoBuzzer buzzer;

void setup() {
  //Inizialize modulino boards
  Modulino.begin();
  knob.begin();
  knob.set(0);
  buzzer.begin();
  leds.begin();
}

int i;
//Encoder value and button status
int knobPosition = 0;
bool knobClick = false;

void loop() {
  //Acquire current encoder value
  knobPosition = knob.get();

  /*Check if encoder value is outside of range
  and if so keep it in range*/
  if (knobPosition > KNOBMAX) {
    knobPosition = KNOBMAX;
  } else if (knobPosition <= KNOBMIN) {
    knobPosition = KNOBMIN;
  }

  //Set to be lit the LED corresponding to encoder value
  leds.set(knobPosition, RED, 100);

  //Shut of the other LEDs
  for (i = KNOBMIN; i <= KNOBMAX; i++) {
    if (i != knobPosition) {
      leds.clear(i);
    }
  }

  //Lit the LED
  leds.show();

  //Check if encoder button is pressed
  knobClick = knob.isPressed();

  /*If the encoder button is pressed reproduce sound
  with pitch equal to encoder value * 100Hz + 200 Hz*/
  if (knobClick) {
    buzzer.tone(knobPosition * 100 + 200, 10);
  }
}