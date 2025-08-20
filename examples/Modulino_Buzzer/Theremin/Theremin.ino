#include <Arduino_Modulino.h>

ModulinoBuzzer buzzer;
ModulinoDistance distance;
ModulinoKnob encoder;


void setup() {

  Modulino.begin();
  encoder.begin();
  distance.begin();
  buzzer.begin();
}

int pitch = 0;
int noteIndex = 0;

//Notes from C to B with #
int note[] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494};

void loop() {
  pitch = distance.get();
  noteIndex = encoder.get();

  noteIndex = noteIndex % 11;
  if (noteIndex < 0){
      noteIndex = 0;
  }

  buzzer.tone(note[noteIndex] + pitch, 1000);
}