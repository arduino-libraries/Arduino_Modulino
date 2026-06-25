# Arduino Modulino® Library

The **Modulino** library is designed to simplify integration with various Modulino. It supports a variety of modules, such as motion sensors, buttons, buzzers, LED displays, and more, all through an I2C (`Wire`) interface.

## Hardware Compatibility

The library is compatible with Arduino boards that support I2C (`Wire`) communication.

Each Modulino has a fixed I2C address assigned by default. If you wish to change the I2C address, you can refer to the [Utilities](#utilities) section.

## Main Features

The **Modulino** library supports the following hardware modules:

- **Buttons (`ModulinoButtons`)**: Read the state of buttons and control the associated LEDs.
- **Buzzer (`ModulinoBuzzer`)**: Activate and deactivate the buzzer and set its frequency.
- **Distance (`ModulinoDistance`)**: Measures distance using a Time-of-Flight (ToF) sensor (VL53L0x).
- **Joystick (`ModulinoJoystick`)**: Easily navigate, steer, or control movement with smooth analog precision.
- **Knob (`ModulinoKnob`)**: Read the value of a rotary encoder.
- **Latch Relay (`ModulinoLatchRelay`)**: Enables safe and efficient switching of external loads.
- **LED Matrix (`ModulinoLEDMatrix`)**: Customizable visual output ranging from simple indicators to pixel animations.
- **LEDs (`ModulinoPixels`)**: Control RGB LEDs with customizable display modes.
- **Light (`ModulinoLight`)**: Detects IR light, recognizes color, and measures surrounding light levels.
- **Motion (`ModulinoMovement`)**: Interface with the LSM6DSOX IMU sensor to get acceleration values.
- **Temperature & Humidity (`ModulinoThermo`)**: Get temperature and humidity readings from the HS300x sensor.
- **Vibro (`ModulinoVibro`)**: Provides subtle vibration responses for alerts or notifications.

## Library Initialization

To initialize the **Modulino** library, include the header file and call the `begin()` method. This will set up the I2C communication and prepare the library for use with the modules.

```cpp
#include <Arduino_Modulino.h>
Modulino.begin();  // Initialize the Modulino library
```

## Supported Modules

You can initialize a Modulino board easily. The basic setup requires just a call to the `begin()` method for each module. For example:

```cpp
ModulinoType modulino_name;
modulino_name.begin();  // Initialize the ModulinoType module
```

### ModulinoButtons
Manages the state of three buttons and controls their associated LED states. You can read the state of each button and send commands to turn the LEDs on/off.

![Modulino Buttons](./images/modulino-buttons.jpeg)

```cpp
ModulinoButtons buttons;
buttons.begin();
buttons.setLeds(true, false, true);  // Turn on LED 1 and LED 3
```

### ModulinoBuzzer
Allows you to emit sounds through the buzzer. You can set the frequency and duration of the sound.

![Modulino Buzzer](./images/modulino-buzzer.jpg)

```cpp
ModulinoBuzzer buzzer;
buzzer.begin();
buzzer.tone(440, 1000);  // 440Hz frequency for 1000ms
```

### ModulinoDistance
Measures distance using a ToF (Time-of-Flight) sensor.

![Modulino Distance](./images/modulino-distance.jpg)

```cpp
ModulinoDistance distance;
distance.begin();
float distanceValue = distance.get();
```

### ModulinoJoystick
Lets you easily navigate, steer, or control movement in any interactive setup with smooth analog precision.

![Modulino Joystick](./images/modulino-joystick.jpg)

```cpp
ModulinoJoystick joystick;
joystick.begin();
if (joystick.update()) {
  int8_t x = joystick.getX();
  int8_t y = joystick.getY();
}
```

### ModulinoKnob
Manages a rotary encoder, allowing you to read the potentiometer value and check if the knob is pressed.

![Modulino Knob](./images/modulino-knob.jpg)

```cpp
ModulinoKnob knob;
knob.begin();
int16_t value = knob.get();  // Get the value of the encoder
```

### ModulinoLatchRelay
Enables safe and efficient switching of external loads, ideal for automation and energy-saving applications.

![Modulino Latch Relay](./images/modulino-latch-relay.jpg)

```cpp
ModulinoLatchRelay relay;
relay.begin();
relay.set();    // Switch the relay state to on
```

### ModulinoLEDMatrix
Brings your projects to life with customizable visual output, ranging from simple indicators to pixel animations.

![Modulino LED Matrix](./images/modulino-led-matrix.jpg)

```cpp
#include <Modulino_LED_Matrix.h>
// ...
ModulinoLEDMatrix matrix;
matrix.begin();
// Further matrix manipulation ...
```

### ModulinoLight
Detects IR light, recognizes color, and measures surrounding light levels so your projects can automatically adapt to their environment.

![Modulino Light](./images/modulino-light.jpg)

```cpp
ModulinoLight light;
light.begin();
if (light.update()) {
  ModulinoColor color = light.getColor();
}
```

### ModulinoMovement
Interfaces with the LSM6DSOX IMU sensor to get acceleration readings.

![Modulino Movement](./images/modulino-movement.jpg)

```cpp
ModulinoMovement movement;
movement.begin();
float x = movement.getX();
```

### ModulinoPixels
Controls an array of 8 RGB LEDs, allowing you to set the colors and brightness. You can also clear individual LEDs or the entire array.

![Modulino Pixels](./images/modulino-pixels.jpg)

```cpp
ModulinoPixels leds;
leds.set(0, ModulinoColor(255, 0, 0));  // Set the first LED (Position: 0) to red
leds.show();  // Display the LEDs
```

### ModulinoThermo
Reads temperature and humidity data from the HS300x sensor.

![Modulino Thermo](./images/modulino-thermo.jpg)

```cpp
ModulinoThermo thermo;
thermo.begin();
float temperature = thermo.getTemperature();
float humidity = thermo.getHumidity();
```

### ModulinoVibro
Provides subtle vibration responses for alerts, notifications, or immersive experiences.

![Modulino Vibro](./images/modulino-vibro.jpg)

```cpp
ModulinoVibro vibro;
vibro.begin();
vibro.on(1000);  // Vibrate for 1000ms
```

## Example Usage

Here’s an example of how to use some Modulino in a program:

```cpp
// This sketch demonstrates how to use the Modulino library to control buttons, LEDs, and a buzzer.
// It listens for a button press (Button A), turns on the LED 0, and plays a sound through the buzzer when pressed.

#include <Arduino_Modulino.h>

ModulinoButtons buttons;  // Declare a Buttons Modulino
ModulinoPixels leds;      // Declare a Pixels Modulino
ModulinoBuzzer buzzer;    // Declare a Buzzer Modulino

int frequency = 440;      // Set frequency to 440Hz
int duration = 1000;      // Set duration to 1000ms

void setup() {
  Serial.begin(9600);

  Modulino.begin();        // Initialize the Modulino library

  buttons.begin();         // Initialize the Buttons module
  leds.begin();            // Initialize the Pixels module
  buzzer.begin();          // Initialize the Buzzer module
}

void loop() {
  if (buttons.update()) {  // Update the button states
    if (buttons.isPressed(0)) {  // Check if Button A (button 0) is pressed
      Serial.println("Button A pressed!");
      leds.set(0, RED);  // Turn on LED 0 to red
      leds.show();

      buzzer.tone(frequency, duration);  // Play buzzer sound with the specified frequency and duration
      delay(duration);
    } else {
      leds.clear(0); // Turn off LED 0
      leds.show();
    }
  }
}
```

## Examples

The examples folder is organized by Modulino type. Each module has its own folder with example sketches that demonstrate both basic and advanced usage (where applicable).

You can explore the examples [here](../examples).

## Stepper Notes (ModulinoMotors)

- `moveStepper(steps, speedPeriod, releaseDelayMs)` uses `speedPeriod` in 0.1 ms timer ticks (`1..65535`).
- `moveStepperRpm(...)` computes the matching period from RPM and `stepsPerRevolution`.
- The first step of a move is executed immediately on command start. Remaining steps are timer-paced.
- `releaseDelayMs` controls the post-move state:
  - `0`: hold coils energized for holding torque.
  - `1..255`: release coils after the specified delay in milliseconds.
- `hold()` keeps coils energized immediately. `release()` requests a minimal release delay (1 ms).

### Utilities

In the [Utilities](../examples/Utilities) folder, you will find programs designed to help you manage and manipulate the Modulino:

- [AddressChanger](../examples/Utilities/AddressChanger/): This program allows you to change the I2C address of a Modulino module. It’s helpful when you need to reassign addresses to avoid conflicts or organize your I2C network.

## API

The API documentation can be found [here](./api.md).

## License

This library is released under the [MPL-2.0 license](../LICENSE).
