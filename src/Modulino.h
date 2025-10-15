// Copyright (c) 2025 Arduino SA
// SPDX-License-Identifier: MPL-2.0

#ifndef ARDUINO_LIBRARIES_MODULINO_H
#define ARDUINO_LIBRARIES_MODULINO_H

#if defined(ESP32) && defined(BOARD_HAS_PIN_REMAP) && defined(tone)
  #error "The current configuration is unsupported, switch Pin Numbering to "By GPIO number" or #undef tone and #undef noTone in the beginning of your sketch."
  #error "Learn more at: https://support.arduino.cc/hc/en-us/articles/10483225565980-Select-pin-numbering-for-Nano-ESP32-in-Arduino-IDE"
#endif

#include "Wire.h"
#include <vl53l4cd_class.h>  // from stm32duino
#include <vl53l4ed_class.h>  // from stm32duino
#include "Arduino_LSM6DSOX.h"
#include <Arduino_LPS22HB.h>
#include <Arduino_HS300x.h>
//#include <SE05X.h>  // need to provide a way to change Wire object

#ifndef ARDUINO_API_VERSION
#define PinStatus     uint8_t
#define HardwareI2C   TwoWire
#endif

void __increaseI2CPriority();

class ModulinoClass {
public:
#if defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_NANO_R4) || defined(ARDUINO_UNO_Q)
  void begin(HardwareI2C& wire = Wire1) {
#else
  void begin(HardwareI2C& wire = Wire) {
#endif

#ifdef ARDUINO_UNOR4_WIFI
    // unlock Wire1 bus at begin since we don't know the state of the system
    pinMode(WIRE1_SCL_PIN, OUTPUT);
    for (int i = 0; i < 20; i++) {
      digitalWrite(WIRE1_SCL_PIN, HIGH);
      digitalWrite(WIRE1_SCL_PIN, LOW);
    }
#endif
    _wire = &wire;
    _wire->begin();
    _wire->setClock(100000);
    __increaseI2CPriority();
  }
  friend class Module;
protected:
  HardwareI2C* _wire;
};

extern ModulinoClass Modulino;

class Module : public Printable {
public:
  Module(uint8_t address = 0xFF, const char* name = "")
    : address(address), name((char *)name) {}
  virtual ~Module() {}  
  bool begin() {
    if (address >= 0x7F) {
      address = discover() / 2;  // divide by 2 to match address in fw main.c
    }
    return (address < 0x7F);
  }
  virtual uint8_t discover() {
    return 0xFF;
  }
  operator bool() {
    return address < 0x7F;
  }
  static HardwareI2C* getWire() {
    return Modulino._wire;
  }
  bool read(uint8_t* buf, int howmany) {
    if (address >= 0x7F) {
      return false;
    }
    Modulino._wire->requestFrom(address, howmany + 1);
    auto start = millis();
    while ((Modulino._wire->available() == 0) && (millis() - start < 100)) {
      delay(1);
    }
    if (Modulino._wire->available() < howmany) {
      return false;
    }
    pinstrap_address = Modulino._wire->read();
    for (int i = 0; i < howmany; i++) {
      buf[i] = Modulino._wire->read();
    }
    while (Modulino._wire->available()) {
      Modulino._wire->read();
    }
    return true;
  }
  bool write(uint8_t* buf, int howmany) {
    if (address >= 0x7F) {
      return false;
    }
    Modulino._wire->beginTransmission(address);
    for (int i = 0; i < howmany; i++) {
      Modulino._wire->write(buf[i]);
    }
    Modulino._wire->endTransmission();
    return true;
  }
  bool nonDefaultAddress() {
    return (pinstrap_address != address);
  }
  virtual size_t printTo(Print& p) const {
    return p.print(name);
  }
  bool scan(uint8_t addr) {
    Modulino._wire->beginTransmission(addr / 2);  // multply by 2 to match address in fw main.c
    auto ret = Modulino._wire->endTransmission();
    if (ret == 0) {
      // could also ask for 1 byte and check if it's truely a modulino of that kind
      return true;
    }
    return false;
  }
private:
  uint8_t address;
  uint8_t pinstrap_address;
  char* name;
};

class ModulinoButtons : public Module {
public:
  ModulinoButtons(uint8_t address = 0xFF)
    : Module(address, "BUTTONS") {}
  PinStatus isPressed(int index) {
    return last_status[index] ? HIGH : LOW;
  }
  PinStatus isPressed(char button) {
    int index = buttonToIndex(button);
    if (index < 0) return LOW;
    return isPressed(index);
  }
  PinStatus isPressed(const char *button) {
    if (button == nullptr || button[0] == '\0' || button[1] != '\0') {
      return LOW;
    }
    return isPressed(button[0]);
  }
  bool update() {
    uint8_t buf[3];
    auto res = read((uint8_t*)buf, 3);
    auto ret = res && (buf[0] != last_status[0] || buf[1] != last_status[1] || buf[2] != last_status[2]);
    last_status[0] = buf[0];
    last_status[1] = buf[1];
    last_status[2] = buf[2];
    return ret;
  }
  void setLeds(bool a, bool b, bool c) {
    uint8_t buf[3];
    buf[0] = a;
    buf[1] = b;
    buf[2] = c;
    write((uint8_t*)buf, 3);
    return;
  }
  virtual uint8_t discover() {
    for (unsigned int i = 0; i < sizeof(match)/sizeof(match[0]); i++) {
      if (scan(match[i])) {
        return match[i];
      }
    }
    return 0xFF;
  }
private:
  bool last_status[3];
  int buttonToIndex(char button) {
    switch (toupper(button)) {
      case 'A': return 0;
      case 'B': return 1;
      case 'C': return 2;
      default:  return -1;
    }
  }
protected:
  uint8_t match[1] = { 0x7C };  // same as fw main.c
};

class ModulinoBuzzer : public Module {
public:
  ModulinoBuzzer(uint8_t address = 0xFF)
    : Module(address, "BUZZER") {}
  void (tone)(size_t freq, size_t len_ms) {
    uint8_t buf[8];
    memcpy(&buf[0], &freq, 4);
    memcpy(&buf[4], &len_ms, 4);
    write(buf, 8);
  }
  void (noTone)() {
    uint8_t buf[8];
    memset(&buf[0], 0, 8);
    write(buf, 8);
  }
  virtual uint8_t discover() {
    for (unsigned int i = 0; i < sizeof(match)/sizeof(match[0]); i++) {
      if (scan(match[i])) {
        return match[i];
      }
    }
    return 0xFF;
  }
protected:
  uint8_t match[1] = { 0x3C };  // same as fw main.c
};

class ModulinoColor {
public:
  ModulinoColor(uint8_t r, uint8_t g, uint8_t b)
    : r(r), g(g), b(b) {}
  operator uint32_t() {
    return (b << 8 | g << 16 | r << 24);
  }
private:
  uint8_t r, g, b;
};

class ModulinoPixels : public Module {
public:
  ModulinoPixels(uint8_t address = 0xFF)
    : Module(address, "LEDS") {
    memset((uint8_t*)data, 0xE0, NUMLEDS * 4);
  }
  void set(int idx, ModulinoColor rgb, uint8_t brightness = 25) {
    if (idx < NUMLEDS) {
      uint8_t _brightness = map(brightness, 0, 100, 0, 0x1F);
      data[idx] = (uint32_t)rgb | _brightness | 0xE0;
    }
  }
  void set(int idx, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness = 5) {
    set(idx, ModulinoColor(r,g,b), brightness);
  }
  void clear(int idx) {
    set(idx, ModulinoColor(0,0,0), 0);
  }
  void clear() {
    memset((uint8_t*)data, 0xE0, NUMLEDS * 4);
  }
  void show() {
    write((uint8_t*)data, NUMLEDS * 4);
  }
  virtual uint8_t discover() {
    for (unsigned int i = 0; i < sizeof(match)/sizeof(match[0]); i++) {
      if (scan(match[i])) {
        return match[i];
      }
    }
    return 0xFF;
  }
private:
  static const int NUMLEDS = 8;
  uint32_t data[NUMLEDS];
protected:
  uint8_t match[1] = { 0x6C };
};


class ModulinoKnob : public Module {
public:
  ModulinoKnob(uint8_t address = 0xFF)
    : Module(address, "ENCODER") {}
  bool begin() {
    auto ret = Module::begin();
    if (ret) {
      auto _val = get();
      _lastPosition = _val;
      _lastDebounceTime = millis();
      set(100);
      if (get() != 100) {
        _bug_on_set = true;
        set(-_val);
      } else {
        set(_val);
      }
    }
    return ret;
  }
  int16_t get() {
    uint8_t buf[3];
    auto res = read(buf, 3);
    if (res == false) {
      return 0;
    }
    _pressed = (buf[2] != 0);
    int16_t ret = buf[0] | (buf[1] << 8);
    return ret;
  }
  void set(int16_t value) {
    if (_bug_on_set) {
      value = -value;
    }
    uint8_t buf[4];
    memcpy(buf, &value, 2);
    write(buf, 4);
  }
  bool isPressed() {
    get();
    return _pressed;
  }
  int8_t getDirection() {
    unsigned long now = millis();
    if (now - _lastDebounceTime < DEBOUNCE_DELAY) {
      return 0;
    }
    int16_t current = get();
    int8_t direction = 0;
    if (current > _lastPosition) {
      direction = 1;
    } else if (current < _lastPosition) {
      direction = -1;
    }
    if (direction != 0) {
      _lastDebounceTime = now;
      _lastPosition = current;
    }
    return direction;
  }
  virtual uint8_t discover() {
    for (unsigned int i = 0; i < sizeof(match)/sizeof(match[0]); i++) {
      if (scan(match[i])) {
        return match[i];
      }
    }
    return 0xFF;
  }
private:
  bool _pressed = false;
  bool _bug_on_set = false;
  int16_t _lastPosition = 0;
  unsigned long _lastDebounceTime = 0;
  static constexpr unsigned long DEBOUNCE_DELAY = 30;
protected:
  uint8_t match[2] = { 0x74, 0x76 };
};

extern ModulinoColor RED;
extern ModulinoColor BLUE;
extern ModulinoColor GREEN;
extern ModulinoColor VIOLET;
extern ModulinoColor WHITE;

class ModulinoMovement : public Module {
public:
  bool begin() {
    if (_imu == nullptr) {
      _imu = new LSM6DSOXClass(*((TwoWire*)getWire()), 0x6A);
    }
    initialized = _imu->begin();
    __increaseI2CPriority();
    return initialized != 0;
  }
  operator bool() {
    return (initialized != 0);
  }
  int update() {
    if (initialized) {
      int accel = _imu->readAcceleration(x, y, z);
      int gyro = _imu->readGyroscope(roll, pitch, yaw);
      return accel && gyro;
    }
    return 0;
  }
  int available() {
    if (initialized) {
      return _imu->accelerationAvailable() && _imu->gyroscopeAvailable();
    }
    return 0;
  }
  float getX() {
    return x;
  }
  float getY() {
    return y;
  }
  float getZ() {
    return z;
  }
  float getRoll() {
    return roll;
  }
  float getPitch() {
    return pitch;
  }
  float getYaw() {
    return yaw;
  }
private:
  LSM6DSOXClass* _imu = nullptr;
  float x,y,z;
  float roll,pitch,yaw; //gx, gy, gz
  int initialized = 0;
};

class ModulinoThermo: public Module {
public:
  bool begin() {
    if (_sensor == nullptr) {
      _sensor = new HS300xClass(*((TwoWire*)getWire()));
    }
    initialized = _sensor->begin();
    __increaseI2CPriority();
    return initialized;
  }
  operator bool() {
    return (initialized != 0);
  }
  float getHumidity() {
    if (initialized) {
      return _sensor->readHumidity();
    }
    return 0;
  }
  float getTemperature() {
    if (initialized) {
      return _sensor->readTemperature();
    }
    return 0;
  }
private:
  HS300xClass* _sensor = nullptr;
  int initialized = 0;
};

class ModulinoPressure : public Module {
public:
  bool begin() {
    if (_barometer == nullptr) {
      _barometer = new LPS22HBClass(*((TwoWire*)getWire()));
    }
    initialized = _barometer->begin();
    if (initialized == 0) {
      // unfortunately LPS22HBClass calles Wire.end() on failure, restart it
      getWire()->begin();
    }
    __increaseI2CPriority();
    return initialized != 0;
  }
  operator bool() {
    return (initialized != 0);
  }
  float getPressure() {
    if (initialized) {
      return _barometer->readPressure();
    }
    return 0;
  }
  float getTemperature() {
    if (initialized) {
      return _barometer->readTemperature();
    }
    return 0;
  }
private:
  LPS22HBClass* _barometer = nullptr;
  int initialized = 0;
};

class ModulinoLight : public Module {

};

class _distance_api {
public:
  _distance_api(VL53L4CD* sensor) : sensor(sensor) {
    isVL53L4CD = true;
  };
  _distance_api(VL53L4ED* sensor) : sensor(sensor) {};
  uint8_t setRangeTiming(uint32_t timing_budget_ms, uint32_t inter_measurement_ms) {
    if (isVL53L4CD) {
      return ((VL53L4CD*)sensor)->VL53L4CD_SetRangeTiming(timing_budget_ms, inter_measurement_ms);
    } else {
      return ((VL53L4ED*)sensor)->VL53L4ED_SetRangeTiming(timing_budget_ms, inter_measurement_ms);
    }
  }
  uint8_t startRanging() {
    if (isVL53L4CD) {
      return ((VL53L4CD*)sensor)->VL53L4CD_StartRanging();
    } else {
      return ((VL53L4ED*)sensor)->VL53L4ED_StartRanging();
    }
  }
  uint8_t checkForDataReady(uint8_t* p_is_data_ready) {
    if (isVL53L4CD) {
      return ((VL53L4CD*)sensor)->VL53L4CD_CheckForDataReady(p_is_data_ready);
    } else {
      return ((VL53L4ED*)sensor)->VL53L4ED_CheckForDataReady(p_is_data_ready);
    }
  }
  uint8_t clearInterrupt() {
    if (isVL53L4CD) {
      return ((VL53L4CD*)sensor)->VL53L4CD_ClearInterrupt();
    } else {
      return ((VL53L4ED*)sensor)->VL53L4ED_ClearInterrupt();
    }
  }
  uint8_t getResult(void* result) {
    if (isVL53L4CD) {
      return ((VL53L4CD*)sensor)->VL53L4CD_GetResult((VL53L4CD_Result_t*)result);
    } else {
      return ((VL53L4ED*)sensor)->VL53L4ED_GetResult((VL53L4ED_ResultsData_t*)result);
    }
  }
private:
  void* sensor;
  bool isVL53L4CD = false;
};

class ModulinoDistance : public Module {
public:
  bool begin() {
    // try scanning for 0x29 since the library contains a while(true) on begin()
    getWire()->beginTransmission(0x29);
    if (getWire()->endTransmission() != 0) {
      return false;
    }
    tof_sensor = new VL53L4CD((TwoWire*)getWire(), -1);
    auto ret = tof_sensor->InitSensor();
    if (ret != VL53L4CD_ERROR_NONE) {
      delete tof_sensor;
      tof_sensor = nullptr;
      tof_sensor_alt = new VL53L4ED((TwoWire*)getWire(), -1);
      ret = tof_sensor_alt->InitSensor();
      if (ret == VL53L4ED_ERROR_NONE) {
        api = new _distance_api(tof_sensor_alt);
      } else {
        delete tof_sensor_alt;
        tof_sensor_alt = nullptr;
        return false;
      }
    } else {
      api = new _distance_api(tof_sensor);
    }

    __increaseI2CPriority();
    api->setRangeTiming(20, 0);
    api->startRanging();
    return true;
  }
  operator bool() {
    return (api != nullptr);
  }
  bool available() {
    if (api == nullptr) {
      return false;
    }
    
    uint8_t NewDataReady = 0;
    api->checkForDataReady(&NewDataReady);
    if (NewDataReady) {
      api->clearInterrupt();
      api->getResult(&results);
    }
    if (results.range_status == 0) {
      internal = results.distance_mm;
    } else {
      internal = NAN;
    }
    return !isnan(internal);
  }
  float get() {
    return internal;
  }
private:
  VL53L4CD* tof_sensor = nullptr;
  VL53L4ED* tof_sensor_alt = nullptr;
  VL53L4CD_Result_t results;
  //VL53L4ED_ResultsData_t results;
  float internal = NAN;
  _distance_api* api = nullptr;
};

#endif
