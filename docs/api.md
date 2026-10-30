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
| `class` [`ModulinoLoadCell`](#modulinoloadcell) | Handles functionality Modulino Load Cell, reading the weight from one or two strain gauge load cells with tare and calibration stored in the module. |

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

### ModulinoLoadCell
Represents Modulino Load Cell module. Channel A (J5, gain 128) is `ModulinoLoadCell::CHANNEL_A`, channel B (J6, gain 32) is `ModulinoLoadCell::CHANNEL_B`; every `channel` argument defaults to A.

#### Methods
- **`bool begin()`**
Finds the module, checks that its firmware speaks the protocol of this library and brings it to its power-on state: stored calibration, channel A, 10 SPS. `false` if missing or if its firmware must be updated (FirmwareUpdater example).
- **`bool available(uint8_t channel)`**
Reads the module and returns `true` if a new sample arrived on the channel since the last call.
- **`float getWeight(uint8_t channel)`**
Weight in grams, filtered and tared. `NAN` if no scale is set.
- **`int32_t getRaw(uint8_t channel)`**
Last raw converter sample (signed 24 bit).
- **`bool setLoadCell(float mVperV, uint32_t capacityGrams, uint8_t channel)`**
Sets the scale from the cell datasheet (rated output and capacity). Accuracy is the cell tolerance until `calibrate()` is used.
- **`bool tare(uint8_t channel, uint8_t samples = 10)`**
Sets the current load as zero, averaging `samples` samples on the module. The channel must be sampled in the current mode: for channel B, `setMode(Mode::B)` or `Mode::A_AND_B` first, otherwise `Result::CHANNEL_INACTIVE`.
- **`bool calibrate(float knownGrams, uint8_t channel, uint8_t samples = 10)`**
Computes the scale from a known weight placed on the cell after `tare()`.
- **`bool saveCalibration()` / `bool eraseCalibration()`**
Stores in, or erases from, the module the offset, scale, capacity and filter of both channels.
- **`bool hasStoredCalibration(uint8_t channel)`**
`true` if the module holds a stored calibration for the channel.
- **`bool setFilter(uint16_t ms, uint8_t channel)`**
Low-pass filter on the weight, time constant in ms (0 = off).
- **`bool setOffset(int32_t counts, uint8_t channel)`**, **`bool setScale(float countsPerGram, uint8_t channel)`**, **`bool setCapacity(uint32_t grams, uint8_t channel)`**
Set the calibration values directly. `readSettings(channel)` reads them back for `getOffset()`, `getScale()`, `getCapacity()`, `getFilter()`.
- **`bool isOverCapacity(uint8_t channel)`**, **`bool isSaturated(uint8_t channel)`**, **`bool isWeightValid(uint8_t channel)`**
Load above the rated capacity; input out of range (cell disconnected or miswired); scale set.
- **`bool setMode(ModulinoLoadCell::Mode mode)`**
`Mode::A` (default), `Mode::B`, or `Mode::A_AND_B` (each channel about 1 sample/s at 10 SPS, 7.7 at 80 SPS).
- **`bool setRate(ModulinoLoadCell::Rate rate)`**
`Rate::SPS_10` (default, lowest noise, rejects 50/60 Hz mains pickup) or `Rate::SPS_80` (no mains rejection).
- **`bool powerDown()` / `bool powerUp()`**
Switch the converter and the bridge excitation off and on.
- **`ModulinoLoadCell::Result getLastResult()`**
Outcome of the last command, for example `Result::SIGNAL_TOO_SMALL` after a calibration with too light a weight.
- **`String getFirmwareVersion()`**
Firmware version of the module.

---

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
