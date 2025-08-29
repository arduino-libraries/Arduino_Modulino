#include "Modulino.h"

#define FREQUENCY_LOWEST 100      //Lowest frequency in hertz
#define FREQUENCY_HIGHEST 8000    //Highest frequency in hertz
#define FREQUENCY_STEP 10         //Each iteration changes frequency by this number

ModulinoBuzzer buzzer;

int frequency = FREQUENCY_LOWEST;
String frequency_text = String(frequency) + " Hz";

void setup() {
  Serial.begin(115200);

  //Inizialise modulinos
  Modulino.begin();
  buzzer.begin();

  delay(100);
}

void loop() {
  //Increase played frequency by the desired step.
  //Start again if highest frequency has been played
  frequency = frequency + FREQUENCY_STEP;
  if (frequency > FREQUENCY_HIGHEST) {
    frequency = FREQUENCY_LOWEST;
  }

  //Play the frequency on the buzzer
  buzzer.tone(frequency, 1000);

  //Convert the frequency value to string and add Hz
  frequency_text = String(frequency) + " Hz";
  Serial.println(frequency_text);
}
