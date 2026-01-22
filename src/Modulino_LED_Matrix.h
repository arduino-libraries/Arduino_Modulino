#pragma once

#include "Arduino.h"
#include "Wire.h"
#include "gallery.h"

#define NUM_LEDS    96

#if __has_include("ArduinoGraphics.h")
#include <ArduinoGraphics.h>
#define MATRIX_WITH_ARDUINOGRAPHICS
#endif

// TODO: this is dangerous, use with care
#define loadSequence(frames)                    loadWrapper(frames, sizeof(frames))
#define renderBitmap(bitmap, rows, columns)     loadPixels(&bitmap[0][0], rows*columns)
#define endTextAnimation(scrollDirection, anim) endTextToAnimationBuffer(scrollDirection, anim ## _buf, sizeof(anim ## _buf), anim ## _buf_used)
#define loadTextAnimationSequence(anim)         loadWrapper(anim ## _buf, anim ## _buf_used)

#if defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_NANO_R4) || defined(ARDUINO_UNO_Q)
#define DEFAULT_WIRE Wire1
#else
#define DEFAULT_WIRE Wire
#endif

#define DEFAULT_ADDRESS 0x39

class ModulinoLEDMatrix
#ifdef MATRIX_WITH_ARDUINOGRAPHICS
    : public ArduinoGraphics
#endif
     {

public:
    ModulinoLEDMatrix(uint8_t address = DEFAULT_ADDRESS, HardwareI2C& wire = DEFAULT_WIRE)
    #ifdef MATRIX_WITH_ARDUINOGRAPHICS
        : ArduinoGraphics(canvasWidth, canvasHeight)
    #endif
    {
        _address = address;
        _wire = &wire;
    }

    ModulinoLEDMatrix(HardwareI2C& wire, uint8_t address = DEFAULT_ADDRESS)
    #ifdef MATRIX_WITH_ARDUINOGRAPHICS
        : ArduinoGraphics(canvasWidth, canvasHeight)
    #endif
    {
        _address = address;
        _wire = &wire;
    }

    // TODO: find a better name
    // autoscroll will be slower than calling next() at precise times
    void autoscroll(uint32_t interval_ms) {
        _interval = interval_ms;
    }
    int begin() {
        bool rv = true;
        _wire->begin();
        return rv;
    }
    void next() {
        uint32_t frame[3];
        frame[0] = *(_frames+(_currentFrame*4)+0);
        frame[1] = *(_frames+(_currentFrame*4)+1);
        frame[2] = *(_frames+(_currentFrame*4)+2);

        uint8_t data[12];
        prepareFrame(frame, data);

        _interval = *(_frames+(_currentFrame*4)+3);
        _currentFrame = (_currentFrame + 1) % _framesCount;
        if(_currentFrame == 0){
            if(!_loop){
                _interval = 0;
            }
            if(_callBack != nullptr){
                _callBack();
            }
            _sequenceDone = true;
        }
        _wire->beginTransmission(_address);
        _wire->write(data, 12);
        _wire->endTransmission();
    }
    void loadFrame(const uint32_t buffer[3]){
        uint32_t tempBuffer[][4] = {{
            buffer[0], buffer[1], buffer[2], 0
        }};
        loadSequence(tempBuffer);
        next();
        _interval = 0;
    }
    void renderFrame(uint8_t frameNumber){
        _currentFrame = frameNumber % _framesCount;
        next();
        _interval = 0;
    }
    void play(bool looping = false){
        _loop = looping;
        _sequenceDone = false;
        do {
            next();
            delay(_interval);
        } while (_sequenceDone == false);
    }
    bool sequenceDone(){
        if(_sequenceDone){
            _sequenceDone = false;
            return true;
        }
        return false;
    }

    static void loadPixelsToBuffer(uint8_t* arr, size_t size, uint32_t* dst) {
        uint32_t partialBuffer = 0;
        uint8_t pixelIndex = 0;
        uint8_t *frameP = arr;
        uint32_t *frameHolderP = dst;
        while (pixelIndex < size) {
            partialBuffer |= *frameP++;
            if ((pixelIndex + 1) % 32 == 0) {
                *(frameHolderP++) = partialBuffer;
            }
            partialBuffer = partialBuffer << 1;
            pixelIndex++;
        }
    }

    void loadPixels(uint8_t *arr, size_t size){
        loadPixelsToBuffer(arr, size, _frameHolder);
        loadFrame(_frameHolder);
    };

    void loadWrapper(const uint32_t frames[][4], uint32_t howMany) {
        _currentFrame = 0;
        _frames = (uint32_t*)frames;
        _framesCount = (howMany / 4) / sizeof(uint32_t);
    }
    // WARNING: callbacks are fired from ISR. The execution time will be limited.
    void setCallback(voidFuncPtr callBack){
        _callBack = callBack;
    }

    void clear() {
        const uint32_t fullOff[] = {
            0x00000000,
            0x00000000,
            0x00000000
        };
        loadFrame(fullOff);
#ifdef MATRIX_WITH_ARDUINOGRAPHICS
        memset(_canvasBuffer, 0, sizeof(_canvasBuffer));
#endif
    }


#ifdef MATRIX_WITH_ARDUINOGRAPHICS
    virtual void set(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
      if (y >= canvasHeight || x >= canvasWidth || y < 0 || x < 0) {
        return;
      }
      // the r parameter is (mis)used to set the character to draw with
      _canvasBuffer[y][x] = (r | g | b) > 0 ? 1 : 0;
    }

    void endText(int scrollDirection = NO_SCROLL) {
      ArduinoGraphics::endText(scrollDirection);
      renderBitmap(_canvasBuffer, canvasHeight, canvasWidth);
    }

    // display the drawing or capture it to buffer when rendering dynamic anymation
    void endDraw() {
      ArduinoGraphics::endDraw();

      if (!captureAnimation) {
        renderBitmap(_canvasBuffer, canvasHeight, canvasWidth);
      } else {
        if (captureAnimationHowManyRemains >= 4) {
          loadPixelsToBuffer(&_canvasBuffer[0][0], sizeof(_canvasBuffer), captureAnimationFrame);
          captureAnimationFrame[3] = _textScrollSpeed;
          captureAnimationFrame += 4;
          captureAnimationHowManyRemains -= 16;
        }
      }
    }

    void endTextToAnimationBuffer(int scrollDirection, uint32_t frames[][4], uint32_t howManyMax, uint32_t& howManyUsed) {
      captureAnimationFrame = &frames[0][0];
      captureAnimationHowManyRemains = howManyMax;

      captureAnimation = true;
      ArduinoGraphics::textScrollSpeed(0);
      ArduinoGraphics::endText(scrollDirection);
      ArduinoGraphics::textScrollSpeed(_textScrollSpeed);
      captureAnimation = false;
        
      howManyUsed = howManyMax - captureAnimationHowManyRemains;
    }

    void textScrollSpeed(unsigned long speed) {
      ArduinoGraphics::textScrollSpeed(speed);
      _textScrollSpeed = speed;
    }

  private:
    uint32_t* captureAnimationFrame = nullptr;
    uint32_t captureAnimationHowManyRemains = 0;
    bool captureAnimation = false;
    static const byte canvasWidth = 12;
    static const byte canvasHeight = 8;
    uint8_t _canvasBuffer[canvasHeight][canvasWidth] = {{0}};
    unsigned long _textScrollSpeed = 100;
#endif

private:

    void prepareFrame(uint32_t* frame, uint8_t* data) {
        memset(data, 0, 12);
        for (int x = 0; x < 12; x++) {
            for (int y = 0; y < 8; y++) {
                int k = y * 12 + x;
                int w_idx = k / 32;
                int b_idx = 31 - (k % 32);

                if ((frame[w_idx] >> b_idx) & 1) {
                    data[x] |= (1 << y);
                }
            }
        }
    }

    int _currentFrame = 0;
    uint32_t _frameHolder[3];
    uint32_t* _frames;
    uint32_t _framesCount;
    uint32_t _interval = 0;
    uint32_t _lastInterval = 0;
    bool _loop = false;
    bool _sequenceDone = false;
    voidFuncPtr _callBack = nullptr;
    HardwareI2C* _wire;
    uint8_t _address = DEFAULT_ADDRESS;
};
