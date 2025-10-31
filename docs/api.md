# Arduino Modulino Library API

## Summary

 Members                                    | Descriptions
--------------------------------------------|------------------------------------------
| `class` [`ModulinoClass`](#modulinoclass) | The base class for all Modulino components, providing essential functionality and structure for all subsequent Modulino modules.                                  |
| `class` [`ModulinoButtons`](#modulinobuttons) | Handles the functionality of Modulino Buttons, enabling detection of presses and handling input events.                                                |
| `class` [`ModulinoBuzzer`](#modulinobuttons) |Handles the functionality of Modulino Buzzer, enabling the sound generation for feedback or alerts.                                                    |
| `class` [`ModulinoPixels`](#modulinopixels) | Handles the functionality of Modulino Pixels, managing LEDs strip color, status and brightness.                                          |
| `class` [`ModulinoKnob`](#modulinoknob) | Handles the functionality of Modulino Knob, interfacing with the rotary knob position.                                                     |
| `class` [`ModulinoMovement`](#modulinomovement) | Handles the functionality of Modulino Movement,interfacing with the IMU sensor to get acceleration readings. |
| `class` [`ModulinoThermo`](#modulinothermo) | Handles the functionality of Modulino Thermo, managing temperature sensors to provide real-time temperature and humidity readings.                                |
| `class` [`ModulinoDistance`](#modulinodistance) | Handles the functionality of Modulino Distance, enabling distance measurement using ToF (Time-of-Flight) sensors for precise range detection. |

### ModulinoClass

The `ModulinoClass` is the main class that handles the initialization of the I2C communication bus.

#### Methods

- **`void begin(HardwareI2C& wire = Wire)`**  
  Initializes the I2C communication. The default I2C bus is `Wire`, but you can specify another one (e.g., `Wire1`).

---

### ModulinoButtons

Represents a Modulino Buttons module.

#### Methods

- **`PinStatus isPressed(int index)`**  
  Returns the press status (HIGH/LOW) of the button at the specified index (_0-A, 1-B, 2-C_).

- **`PinStatus isPressed(char button)`**  
  Returns the press status (HIGH/LOW) of the button specified by its character ('A', 'B', 'C').

- **`PinStatus isPressed(const char *button)`**
  Returns the press status (HIGH/LOW) of the button specified by its string ("A", "B", "C").

- **`bool update()`**  
  Updates the button status. Returns `true` if the status has changed, `false` otherwise.

- **`void setLeds(bool a, bool b, bool c)`**  
  Sets the LED states. Each argument corresponds to one LED's state (on/off).

---

### ModulinoBuzzer

Represents a Modulino Buzzer module.

#### Methods

- **`void tone(size_t freq, size_t len_ms)`**  
  Plays a tone at the specified frequency (`freq`) and duration (`len_ms`).

- **`void noTone()`**  
  Stops any tone currently playing.

---

### ModulinoPixels

Represents a Modulino Pixels module.

#### Methods

- **`void set(int idx, ModulinoColor rgb, uint8_t brightness = 25)`**  
  Sets the color of the LED at the specified index.

- **`void set(int idx, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness = 5)`**  
  Sets the color of the LED at the specified index using RGB values.

- **`void clear(int idx)`**  
  Clears the LED at the specified index (turns it off).

- **`void clear()`**  
  Clears all LEDs (turns them all off).

- **`void show()`**  
  Updates the LEDs to display the current color data.

---

### ModulinoKnob

Represents a Modulino Knob module.

#### Methods

- **`int16_t get()`**  
  Gets the current value of the knob.

- **`bool isPressed()`**  
  Returns `true` if the button on the knob is pressed, `false` otherwise.

- **`int8_t getDirection()`**
  Returns the direction of the knob rotation.  
  - `1` for clockwise
  - `-1` for counter-clockwise
  - `0` if no movement is detected

- **`void set(int16_t value)`**  
  Sets the knob value.

---

### ModulinoMovement

Represents a Modulino Movement module.

#### Methods

- **`int update()`**  
  Updates the sensor data and reads the acceleration values.

- **`int available()`**  
  Returns `true` if acceleration data is available.

- **`float getX()`**  
  Returns the X-axis acceleration.

- **`float getY()`**  
  Returns the Y-axis acceleration.

- **`float getZ()`**  
  Returns the Z-axis acceleration.

- **`float getRoll()`**
  Returns the angular velocity around X-axis.

- **`float getPitch()`**
  Returns the angular velocity around Y-axis.

- **`float getYaw()`**
  Returns the angular velocity around Z-axis.

---

### ModulinoThermo

Represents a Modulino Thermo module.

#### Methods

- **`float getHumidity()`**  
  Returns the relative humidity reading as a percentage in the range 0 to 100.

- **`float getTemperature()`**  
  Returns the temperature reading in Celsius.

---

### ModulinoDistance

Represents a Modulino Distance module.

#### Methods

- **`available()`**  
  Checks if new distance data is available from the sensor. If data is ready, it reads the data and updates the distance value.  
  `true` if distance data is available, `false` if distance data is not available.

- **`float get()`**  
  Returns the distance measured by the sensor in millimeters.  
  The measured distance in millimeters if available, `NAN` if the distance reading is invalid.

## Constants

- **`ModulinoColor RED`**  
  Represents the color red for Modulino Pixels modules.

- **`ModulinoColor BLUE`**  
  Represents the color blue for Modulino Pixels modules.

- **`ModulinoColor GREEN`**  
  Represents the color green for Modulino Pixels modules.

- **`ModulinoColor VIOLET`**  
  Represents the color violet for Modulino Pixels modules.

- **`ModulinoColor WHITE`**  
  Represents the color white for Modulino Pixels modules.

### ModulinoColor

Represents a color for Modulino Pixels modules.

#### Constructor

- **`ModulinoColor(uint8_t r, uint8_t g, uint8_t b)`**  
  Creates a color with the specified red (`r`), green (`g`), and blue (`b`) components.

## License

This library is released under the [MPL-2.0 license](../LICENSE).
