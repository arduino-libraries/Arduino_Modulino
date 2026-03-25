#include <Wire.h>

bool deviceAvailable(uint8_t address) {
    Wire1.beginTransmission(address);
    if (Wire1.endTransmission() == 0) {
        return true;
    }
    return false;
}

void display(){
    constexpr uint8_t LEDMATRIX_UNO_VERTICAL[] = { 0x0c, 0xff, 0x8d, 0x81, 0xa1, 0xa1, 0xa1, 0xa1, 0x81, 0x87, 0x78, 0x00 };
    Wire1.beginTransmission(0x39);
    Wire1.write(LEDMATRIX_UNO_VERTICAL, sizeof(LEDMATRIX_UNO_VERTICAL));
    Wire1.endTransmission();
}

void setup(){
    Serial.begin(9600);
    while (!Serial) {
        ; // Wait for serial port to connect. Needed for native USB
    }
    delay(1000); // Give some time for the serial monitor to initialize

    Wire1.begin(); // Initialize I2C on the secondary bus
    if(deviceAvailable(0x39)) {
        Serial.println("Device found at address 0x39");
    } else {
        Serial.println("No device found at address 0x39");
        return; // Exit if no device is found
    }

    // Read 4 bytes over I2C from 0x39
    uint8_t buf[4];    
    Wire1.requestFrom(0x39, 4);
    while (Wire1.available()) {
        for (int i = 0; i < 4; i++) {
            buf[i] = Wire1.read();
        }
    }
    Serial.print("Pin strap address: 0x");
    Serial.println(buf[0], HEX);

    Serial.print("Display mode: ");
    for (int i = 1; i < 4; i++) {
        Serial.print((char) buf[i]);
    }
    Serial.println();

    display();
}

void loop() {
    // Nothing to do in the loop
}