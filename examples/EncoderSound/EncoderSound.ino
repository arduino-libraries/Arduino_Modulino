#include "Modulino.h"

#define FREQUENCY_DEFAULT 4000   //Starting frequency in hertz
#define FREQUENCY_LOWEST 1       //Minimum frequency in hertz
#define FREQUENCY_HIGHEST 20000  //Maximum frequency in hertz
#define FREQUENCY_STEP 200       //How much the frequency changes when the knob is turned by 1 click
#define CYCLE_WAIT_MS 25         //Amount of added delay between loop iterations

//Create objects for modulinos
ModulinoKnob encoder;
ModulinoBuzzer buzzer;

//Declare variables for the sketch
int frequency = FREQUENCY_DEFAULT;    //frequency which will be played by the buzzer
int actual_value = FREQUENCY_DEFAULT; //Current encoder value.
int previous_value = actual_value;    //Previous encoder value.
bool mute_status = false;             //To use the encoder button as an MUTE/PLAY button.

String frequency_text = String(frequency) + " Hz";
String encoder_text =  "(encoder value " + String(actual_value) + ")";
String mute_text = "- now playing";

void setup() {
  Serial.begin(115200);

  //Inizialise modulinos
  Modulino.begin();
  buzzer.begin();
  encoder.begin();

  //Wait for modulino encoder inizialization and set starting value
  delay(100);
  encoder.set(FREQUENCY_DEFAULT);
}

void loop() {
  //Acquire encoder value
  actual_value = encoder.get();

  /*Check if the encoder value has changed with respect to previous
    acquisition, if so increase/decrease the encoder value by the
    desired FREQUENCY_STEP, then update the previous value variable
  */
  if (previous_value != actual_value) {
    if (actual_value > previous_value) {
      actual_value = actual_value + FREQUENCY_STEP;
    } else {
      actual_value = actual_value - FREQUENCY_STEP;
    }

    if (actual_value < FREQUENCY_LOWEST) {
      actual_value = FREQUENCY_LOWEST;
      encoder.set(FREQUENCY_LOWEST);
    }
    if (actual_value > FREQUENCY_HIGHEST) {
      actual_value = FREQUENCY_HIGHEST;
      encoder.set(FREQUENCY_HIGHEST);
    }
    encoder.set(actual_value);
    previous_value = actual_value;
  }

  //Check if encoder has been pressed. In that case toggle
  //mute status. Update mute text to diplay the MUTE/PLAY on serial
  if (encoder.isPressed()) {
    mute_status = !mute_status;
    if (mute_status == false) {
      mute_text = "- now playing";
    } else {
      mute_text = "- muted";
    }
  }

  //Play the frequency on the buzzer only if the system is not muted.
  if (mute_status == false) {
    buzzer.tone(actual_value, 1000);
  } else {
    buzzer.noTone();
  }

  //Convert the frequency value to string and add Hz
  frequency_text = String(frequency) + " Hz";

  //Convert the encoder value to string
  encoder_text = "(encoder value " + String(actual_value) + ")";

  //Print each of the sensor values on serial
  Serial.print(frequency_text + " ");
  Serial.print(encoder_text + " ");
  Serial.println(mute_text);

  delay(CYCLE_WAIT_MS);
}
