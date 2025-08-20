/*
 * Modulino - Firmware Updater
 *
 * This example code is in the public domain. 
 * Copyright (c) 2025 Arduino
 * SPDX-License-Identifier: MPL-2.0
 */

#if defined(ARDUINO_UNOWIFIR4) 
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"
#endif

#include <Arduino_Modulino.h>
#include "Wire.h"
#include "fw.h"

// https://www.st.com/resource/en/application_note/an4221-i2c-protocol-used-in-the-stm32-bootloader-stmicroelectronics.pdf

bool flash(const uint8_t* binary, size_t lenght, bool verbose = true);

void setup() {
  Serial.begin(115200);
  Wire1.begin();
  Wire1.setClock(400000);

  // Send reset to the module; remember, connect only ONE module at a time
  if (sendReset() != 0) {
    Serial.println("Send reset failed");
  }

  auto result = flash(node_base_bin, node_base_bin_len);
  
  #if defined(ARDUINO_UNOWIFIR4)
  if (result) {
    matrixInitAndDraw("PASS");
  } else {
    matrixInitAndDraw("FAIL");
  }
  #endif
}

void loop() {
  // put your main code here, to run repeatedly:
}

class SerialVerbose {
  public:
    SerialVerbose(bool verbose) : _verbose(verbose) {}
    int print(String s) {
      if (_verbose) {
        Serial.print(s);
      }
    }
    int println(String s) {
      if (_verbose) {
        Serial.println(s);
      }
    }
    int println(int num, int base) {
      if (_verbose) {
        Serial.println(num, base);
      }
    }
  private:
    bool _verbose;
};

#if defined(ARDUINO_UNOWIFIR4)
ArduinoLEDMatrix matrix;

void matrixInitAndDraw(char* text) {
  matrix.begin();
  matrix.beginDraw();

  matrix.stroke(0xFFFFFFFF);
  matrix.textFont(Font_4x6);
  matrix.beginText(0, 1, 0xFFFFFF);
  matrix.println(text);
  matrix.endText();

  matrix.endDraw();
}
#endif

bool flash(const uint8_t* binary, size_t lenght, bool verbose) {

  SerialVerbose SerialDebug(verbose);

  uint8_t resp_buf[255];
  int resp;
  SerialDebug.println("GET_COMMAND");
  resp = command(0, nullptr, 0, resp_buf, 20, verbose);

  if (resp < 0) {
    SerialDebug.println("Failed :(");
    return false;
  }

  for (int i = 0; i < resp; i++) {
    SerialDebug.println(resp_buf[i], HEX);
  }

  SerialDebug.println("GET_ID");
  resp = command(2, nullptr, 0, resp_buf, 3, verbose);
  for (int i = 0; i < resp; i++) {
    SerialDebug.println(resp_buf[i], HEX);
  }

  SerialDebug.println("GET_ID");
  resp = command(2, nullptr, 0, resp_buf, 3, verbose);
  for (int i = 0; i < resp; i++) {
    SerialDebug.println(resp_buf[i], HEX);
  }

  SerialDebug.println("MASS_ERASE");
  uint8_t erase_buf[3] = { 0xFF, 0xFF, 0x0 };
  resp = command(0x44, erase_buf, 3, nullptr, 0, verbose);
  for (int i = 0; i < resp; i++) {
    SerialDebug.println(resp_buf[i], HEX);
  }

  for (int i = 0; i < lenght; i += 128) {
    SerialDebug.print("WRITE_PAGE ");
    SerialDebug.println(i, HEX);
    uint8_t write_buf[5] = { 8, 0, i / 256, i % 256 };
    resp = command_write_page(0x32, write_buf, 5, &binary[i], 128, verbose);
    for (int i = 0; i < resp; i++) {
      SerialDebug.println(resp_buf[i], HEX);
    }
    delay(10);
  }
  SerialDebug.println("GO");
  uint8_t jump_buf[5] = { 0x8, 0x00,  0x00,  0x00, 0x8 };
  resp = command(0x21, jump_buf, 5, nullptr, 0, verbose);
  return true;
}

int howmany;
int command_write_page(uint8_t opcode, uint8_t* buf_cmd, size_t len_cmd, const uint8_t* buf_fw, size_t len_fw, bool verbose) {

  SerialVerbose SerialDebug(verbose);

  uint8_t cmd[2];
  cmd[0] = opcode;
  cmd[1] = 0xFF ^ opcode;
  Wire1.beginTransmission(100);
  Wire1.write(cmd, 2);
  if (len_cmd > 0) {
    buf_cmd[len_cmd - 1] = 0;
    for (int i = 0; i < len_cmd - 1; i++) {
      buf_cmd[len_cmd - 1] ^= buf_cmd[i];
    }
    Wire1.endTransmission(true);
    Wire1.requestFrom(100, 1);
    auto c = Wire1.read();
    if (c != 0x79) {
      SerialDebug.print("error first ack: ");
      SerialDebug.println(c, HEX);
      return -1;
    }
    Wire1.beginTransmission(100);
    Wire1.write(buf_cmd, len_cmd);
  }
  Wire1.endTransmission(true);
  Wire1.requestFrom(100, 1);
  auto c = Wire1.read();
  if (c != 0x79) {
    while (c == 0x76) {
      delay(10);
      Wire1.requestFrom(100, 1);
      c = Wire1.read();
    }
    if (c != 0x79) {
      SerialDebug.print("error second ack: ");
      SerialDebug.println(c, HEX);
      return -1;
    }
  }
  uint8_t tmpbuf[len_fw + 2] = { len_fw - 1 };
  memcpy(&tmpbuf[1], buf_fw, len_fw);
  for (int i = 0; i < len_fw + 1; i++) {
    tmpbuf[len_fw + 1] ^= tmpbuf[i];
  }
  Wire1.beginTransmission(100);
  Wire1.write(tmpbuf, len_fw + 2);
  Wire1.endTransmission(true);
  Wire1.requestFrom(100, 1);
  c = Wire1.read();
  if (c != 0x79) {
    while (c == 0x76) {
      delay(10);
      Wire1.requestFrom(100, 1);
      c = Wire1.read();
    }
    if (c != 0x79) {
      SerialDebug.print("error: ");
      SerialDebug.println(c, HEX);
      return -1;
    }
  }
final_ack:
  return howmany + 1;
}

int command(uint8_t opcode, uint8_t* buf_cmd, size_t len_cmd, uint8_t* buf_resp, size_t len_resp, bool verbose) {

  SerialVerbose SerialDebug(verbose);

  uint8_t cmd[2];
  cmd[0] = opcode;
  cmd[1] = 0xFF ^ opcode;
  Wire1.beginTransmission(100);
  Wire1.write(cmd, 2);
  if (len_cmd > 0) {
    Wire1.endTransmission(true);
    Wire1.requestFrom(100, 1);
    auto c = Wire1.read();
    if (c != 0x79) {
      Serial.print("error first ack: ");
      Serial.println(c, HEX);
      return -1;
    }
    Wire1.beginTransmission(100);
    Wire1.write(buf_cmd, len_cmd);
  }
  Wire1.endTransmission(true);
  Wire1.requestFrom(100, 1);
  auto c = Wire1.read();
  if (c != 0x79) {
    while (c == 0x76) {
      delay(100);
      Wire1.requestFrom(100, 1);
      c = Wire1.read();
      SerialDebug.println("retry");
    }
    if (c != 0x79) {
      SerialDebug.print("error second ack: ");
      SerialDebug.println(c, HEX);
      return -1;
    }
  }
  int howmany = -1;
  if (len_resp == 0) {
    goto final_ack;
  }
  Wire1.requestFrom(100, len_resp);
  howmany = Wire1.read();
  for (int j = 0; j < howmany + 1; j++) {
    buf_resp[j] = Wire1.read();
  }

  Wire1.requestFrom(100, 1);
  c = Wire1.read();
  if (c != 0x79) {
    SerialDebug.print("error: ");
    SerialDebug.println(c, HEX);
    return -1;
  }
final_ack:
  return howmany + 1;
}

int sendReset() {
  uint8_t buf[3] = { 'D', 'I', 'E' };
  int ret;
  for (int i = 8; i < 0x78; i++) {
    Wire1.beginTransmission(i);
    ret = Wire1.endTransmission();
    if (ret != 2) {
      Wire1.beginTransmission(i);
      Wire1.write(buf, 40);
      ret = Wire1.endTransmission();
      return ret;
    }
  }
  return ret;
}