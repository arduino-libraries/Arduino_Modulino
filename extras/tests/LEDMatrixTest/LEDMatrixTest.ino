#if defined(ARDUINO_UNO_Q)
#include <Arduino_RouterBridge.h>
#define Serial Monitor
#endif

#include <Wire.h>

#define _wire Wire1 // Use the secondary I2C bus on the Arduino 

/* Graphic in 4-bit grayscale */
constexpr uint8_t GRADIENT[] = { 	0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0x01, 0x23, 0x45, 0x67,
									0x89, 0xAB, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0x01, 0x23,
									0x45, 0x67, 0x89, 0xAB, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB};

constexpr uint8_t LEDMATRIX_UNO_VERTICAL[] = { 0x0c, 0xff, 0x8d, 0x81, 0xa1, 0xa1, 0xa1, 0xa1, 0x81, 0x87, 0x78, 0x00 };

constexpr uint8_t DEFAULT_ADDRESS = 0x39;
constexpr uint8_t GRAYSCALE_FRAME_SIZE = 48;
constexpr uint8_t MONOCHROMATIC_FRAME_SIZE = 12;
constexpr char GRAYSCALE_IDENTIFIER[] = "GS4";
constexpr char MONOCHROMATIC_IDENTIFIER[] = "MON";
constexpr uint8_t MODE_IDENTIFIER_SIZE = 3;

enum class DisplayMode {
    MonochromaticVertical,
    MonochromaticHorizontal,
    Grayscale,
    Default = MonochromaticVertical
};

DisplayMode _mode = DisplayMode::MonochromaticVertical;
uint8_t _address = DEFAULT_ADDRESS;

bool deviceAvailable(uint8_t address) {
    _wire.beginTransmission(address);
    if (_wire.endTransmission() == 0) {
        return true;
    }
    return false;
}

void displayMono(uint8_t address){    
    _wire.beginTransmission(address);
    _wire.write(LEDMATRIX_UNO_VERTICAL, sizeof(LEDMATRIX_UNO_VERTICAL));
    _wire.endTransmission();
}

void displayGrayscale(uint8_t address){
    _wire.beginTransmission(address);
    _wire.write(GRADIENT, sizeof(GRADIENT));
    _wire.endTransmission();
}

void setMode(DisplayMode mode){
    Serial.print("Setting mode to: ");
    if(mode == DisplayMode::Grayscale){
        Serial.println("Grayscale");
    } else {
        Serial.println("Monochromatic");
    }
    _mode = mode;
    sendMode();
    delay(1);
}

bool sendMode(){
    DisplayMode modeOnDevice = readMode();
    Serial.print("Current mode on device: ");
    if(modeOnDevice == DisplayMode::Grayscale){
        Serial.println("Grayscale");
    } else {
        Serial.println("Monochromatic");
    }
    size_t bufferSize = modeOnDevice == DisplayMode::Grayscale ? GRAYSCALE_FRAME_SIZE : MONOCHROMATIC_FRAME_SIZE;
    uint8_t buf[bufferSize];
    memset(buf, 0, bufferSize);
    _wire.beginTransmission(_address);

    if(_mode == DisplayMode::Grayscale){
        memcpy(buf, GRAYSCALE_IDENTIFIER, MODE_IDENTIFIER_SIZE);
        _wire.write(buf, sizeof(buf));
    } else {
        memcpy(buf, MONOCHROMATIC_IDENTIFIER, MODE_IDENTIFIER_SIZE);
        _wire.write(buf, sizeof(buf));
    }
    uint8_t result = _wire.endTransmission();
    return result == 0;
}

DisplayMode readMode(){        
    uint8_t buf[MODE_IDENTIFIER_SIZE + 1]; // +1 for pin strap address
    _wire.requestFrom(_address, sizeof(buf));
    size_t index = 0;
    while (_wire.available()) {
        buf[index++] = _wire.read();
    }
    // Skip the first byte (pin strap address)
    if(memcmp(buf + 1, GRAYSCALE_IDENTIFIER, MODE_IDENTIFIER_SIZE) == 0){
        return DisplayMode::Grayscale;
    } else {
        return DisplayMode::MonochromaticVertical;
    }
}

uint8_t getPinstrapAddress(uint8_t address){
    // Read 4 bytes over I2C from the specified address
    uint8_t buf[4];    
    _wire.requestFrom(address, 4);
    while (_wire.available()) {
        for (int i = 0; i < 4; i++) {
            buf[i] = _wire.read();
        }
    }
    return buf[0]; // Byte 0 contains the pin strap address
}

void switchModes(){

    for(int i = 0; i < 20; i++){
        setMode(DisplayMode::MonochromaticVertical);
        DisplayMode mode = readMode();

        if(mode != DisplayMode::MonochromaticVertical){
            Serial.println("❌ Error: Failed to switch to Monochrome mode");
        } else {
            Serial.println("✅ Successfully switched to Monochrome mode");
        }
        setMode(DisplayMode::Grayscale);
        mode = readMode();

        if(mode != DisplayMode::Grayscale){
            Serial.println("❌ Error: Failed to switch to Grayscale mode");
        } else {
            Serial.println("✅ Successfully switched to Grayscale mode");
        }
    }
}

void setup(){
    Serial.begin(9600);
    while (!Serial) {
        ; // Wait for serial port to connect. Needed for native USB
    }
    delay(1000); // Give some time for the serial monitor to initialize

    _wire.begin(); // Initialize I2C on the secondary bus
    if(deviceAvailable(DEFAULT_ADDRESS)) {
        Serial.println("Device found at address 0x39");
    } else {
        Serial.println("No device found at address 0x39");
        return; // Exit if no device is found
    }

    uint8_t pinstrapAddress = getPinstrapAddress(DEFAULT_ADDRESS);
    Serial.print("Pin strap address: 0x");
    Serial.println(pinstrapAddress, HEX);

    switchModes();
    
    setMode(DisplayMode::MonochromaticVertical);
    displayMono(DEFAULT_ADDRESS);
    delay(1000);
    setMode(DisplayMode::Grayscale);
    displayGrayscale(DEFAULT_ADDRESS);
}

void loop() {
    // Nothing to do in the loop
}