#include "Modulino.h"

#define NUMBER_OF_BOARDS 3         //How many boards
#define NUMBER_OF_LED_PER_BOARD 8  //How many leds per board.
#define FIRST_ADDRESS 10           //First address of the chain. Address list should have no missing numbers
#define LAST_ADDRESS 12            //Last address of the chain. Address list should have no missing numbers

//Define a new, custom color
ModulinoColor YELLOW(255, 75, 0);

//Allocate memory for as many boards as desired
ModulinoPixels* boards_chain = (ModulinoPixels*)malloc(sizeof(ModulinoPixels) * NUMBER_OF_BOARDS);

int i = 0;  //Defined to count stuff

void setup() {
  Modulino.begin();  //Initialize library

  //Inizialize each object of the boards with its own address
  for (i = 0; i < NUMBER_OF_BOARDS; i++) {
    boards_chain[i] = ModulinoPixels(FIRST_ADDRESS + i);
    boards_chain[i].begin();
  }
  //Set all leds at brigthness 0, shutting them off
  ledAllsame(boards_chain, NUMBER_OF_BOARDS, NUMBER_OF_LED_PER_BOARD, RED, 0);
}

void loop() {
  //Switch on all leds with the same color at brightness 10
  ledAllsame(boards_chain, NUMBER_OF_BOARDS, NUMBER_OF_LED_PER_BOARD, YELLOW, 10);
  delay(200);
  ledAllsame(boards_chain, NUMBER_OF_BOARDS, NUMBER_OF_LED_PER_BOARD, WHITE, 10);
  delay(200);
  ledAllsame(boards_chain, NUMBER_OF_BOARDS, NUMBER_OF_LED_PER_BOARD, VIOLET, 10);
  delay(200);
  ledAllsame(boards_chain, NUMBER_OF_BOARDS, NUMBER_OF_LED_PER_BOARD, RED, 10);
  delay(200);
  ledAllsame(boards_chain, NUMBER_OF_BOARDS, NUMBER_OF_LED_PER_BOARD, GREEN, 10);
  delay(200);
  ledAllsame(boards_chain, NUMBER_OF_BOARDS, NUMBER_OF_LED_PER_BOARD, BLUE, 10);
  delay(200);

  //From the last led to the first one, one by one, switch color to red and brightness to 100
  for (i = (NUMBER_OF_BOARDS * NUMBER_OF_LED_PER_BOARD - 1); i >= 0; i--) {
    ledOn(boards_chain, NUMBER_OF_BOARDS, NUMBER_OF_LED_PER_BOARD, RED, 100, i);
    delay(25);
  }

  //From the first led to the last one, one by one, switch color to white and brightness to 10
  //shut off all the others so that one led is travelling the chain
  for (i = 0; i < (NUMBER_OF_BOARDS * NUMBER_OF_LED_PER_BOARD); i++) {
    ledOnOthersOff(boards_chain, NUMBER_OF_BOARDS, NUMBER_OF_LED_PER_BOARD, WHITE, 10, i);
    delay(25);
  }

  //Breathe all the leds in blue color
  for (i = 0; i <= 100; i = i + 1) {
    ledAllsame(boards_chain, NUMBER_OF_BOARDS, NUMBER_OF_LED_PER_BOARD, BLUE, i);
    delay(10);
  }
  for (i = 100; i >= 0; i = i - 1) {
    ledAllsame(boards_chain, NUMBER_OF_BOARDS, NUMBER_OF_LED_PER_BOARD, BLUE, i);
    delay(10);
  }
}

//Switch ON all LEDs on all boards
void ledAllsame(ModulinoPixels* board_array, int number_of_boards, int number_of_leds, ModulinoColor rgb, int brightness) {
  int i_board = 0;  //Local variable to count each board
  int i_led = 0;    //Local variable to count each led on each board
  //For each board
  for (int i_board = 0; i_board < number_of_boards; i_board++) {
    //For each led on the current board
    for (i_led = 0; i_led < number_of_leds; i_led++) {
      //Set the current led on the current board to desired color and brightness
      board_array[i_board].set(i_led, rgb, brightness);
    }
    //Update the current board status
    board_array[i_board].show();
  }
}

//Switch ON just the desired LED, do nothing to the others
//Considers the chain of boards as a single, long board
void ledOn(ModulinoPixels* board_array, int number_of_boards,
          int number_of_leds, ModulinoColor rgb, int brightness, int ledOn) {
  int i_board = 0;    //Local variable to count each board
  int i_led = 0;      //Local variable to count each led on each board
  int chain_led = 0;  //Local variable to check if the desired led is reached
  //For each board
  for (int i_board = 0; i_board < number_of_boards; i_board++) {
    //For each led on the current board
    for (i_led = 0; i_led < number_of_leds; i_led++) {
      //Set the current led on the current board to desired color and brightness
      if (chain_led == ledOn) {
        board_array[i_board].set(i_led, rgb, brightness);
      }
      chain_led++;
    }
    //Update the current board status
    board_array[i_board].show();
  }
}

//Switch ON just the desired LED, shut off the others
//Considers the chain of boards as a single, long board
void ledOnOthersOff(ModulinoPixels* board_array, int number_of_boards,
                    int number_of_leds, ModulinoColor rgb, int brightness, int ledOn) {
  int i_board = 0;    //Local variable to count each board
  int i_led = 0;      //Local variable to count each led on each board
  int chain_led = 0;  //Local variable to check if the desired led is reached
  //For each board
  for (int i_board = 0; i_board < number_of_boards; i_board++) {
    //For each led on the current board
    for (i_led = 0; i_led < number_of_leds; i_led++) {
      //Set the current led on the current board to desired color and brightness
      if (chain_led == ledOn) {
        board_array[i_board].set(i_led, rgb, brightness);
      } else {
        //Shut off the other leds
        board_array[i_board].set(i_led, RED, 0);
      }
      chain_led++;
    }
    //Update the current board status
    board_array[i_board].show();
  }
}
