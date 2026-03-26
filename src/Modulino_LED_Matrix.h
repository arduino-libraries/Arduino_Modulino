#pragma once

#include "Arduino.h"
#include "Wire.h"

#if __has_include("ArduinoGraphics.h")
#include <ArduinoGraphics.h>
#define MATRIX_WITH_ARDUINOGRAPHICS
#endif

#if defined(ARDUINO_UNOR4_WIFI) || defined(ARDUINO_NANO_R4) || defined(ARDUINO_UNO_Q)
#define DEFAULT_WIRE Wire1
#else
#define DEFAULT_WIRE Wire
#endif

constexpr uint8_t NUM_LEDS = 96;
constexpr uint8_t DEFAULT_ADDRESS = 0x39;
constexpr uint8_t GRAYSCALE_FRAME_SIZE = 48;
constexpr uint8_t MONOCHROMATIC_FRAME_SIZE = 12;
constexpr size_t DURATION_SIZE = sizeof(uint32_t);
constexpr size_t MONOCHROMATIC_ANIMATION_FRAME_SIZE = MONOCHROMATIC_FRAME_SIZE + DURATION_SIZE;
constexpr char GRAYSCALE_IDENTIFIER[] = "GS4";
constexpr char MONOCHROMATIC_IDENTIFIER[] = "MON";
constexpr uint8_t MODE_IDENTIFIER_SIZE = 3;

enum class DisplayMode {
    MonochromaticVertical,
    MonochromaticHorizontal,
    Grayscale,
    Default = MonochromaticVertical
};

class ModulinoLEDMatrix
#ifdef MATRIX_WITH_ARDUINOGRAPHICS
    : public ArduinoGraphics
#endif
     {

public:
    ModulinoLEDMatrix(uint8_t address = DEFAULT_ADDRESS, HardwareI2C& wire = DEFAULT_WIRE, DisplayMode mode = DisplayMode::Default)
    #ifdef MATRIX_WITH_ARDUINOGRAPHICS
        : ArduinoGraphics(canvasWidth, canvasHeight)
    #endif
    {
        _address = address;
        _wire = &wire;
        _mode = mode;
    }

    ModulinoLEDMatrix(HardwareI2C& wire, uint8_t address = DEFAULT_ADDRESS, DisplayMode mode = DisplayMode::Default)
    #ifdef MATRIX_WITH_ARDUINOGRAPHICS
        : ArduinoGraphics(canvasWidth, canvasHeight)
    #endif
    {
        _address = address;
        _wire = &wire;
        _mode = mode;
    }

    /**
     * Sets the display mode for the LED matrix display.
     * @param mode The desired display mode (MonochromaticVertical, MonochromaticHorizontal, grayscale)
     */
    void setMode(DisplayMode mode){
        _mode = mode;
        sendMode();
        delay(1); // Give some time for the device to switch mode before sending frames
    }

    /**
     * Initializes the I2C communication with the LED matrix display
     */
    int begin() {
        if (_initialized) {
            return 1; // Already initialized
        }
        _wire->begin();
        _initialized = true;
        bool success = sendMode();

        if (!success) {
            _initialized = false;
            _wire->end();
        }
        return success ? 1 : 0;
    }

    /**
     * Advances to the next frame in the sequence.
     * Prepares and sends the frame data to the LED matrix display.
     */
    void nextFrame() {        
        renderFrame();

        _currentFrameNumber = (_currentFrameNumber + 1) % _framesCount;
        if(_currentFrameNumber == 0){
            _sequenceDone = true;
            
            if(_sequenceDoneCallBack != nullptr){
                _sequenceDoneCallBack();
            }
        }
    }

    /**
     * Sets the current frame to be displayed on the LED matrix.
     * The frame data is provided as a N-element array of 32-bit integers.
     * The data is expected to be in big-endian format.
     * @param buffer Pointer to an array of N uint32_t containing the frame data.
     */
    template<size_t N>
    void setFrame(const uint32_t(&buffer)[N]){
        size_t frameSize = expectedFrameSize();
        uint8_t data[frameSize];
        convert32to8bit(buffer, N, data, frameSize);
        prepareFrame(data);
        sendFrame(data, frameSize);
    }

    /**
     * Sets the current frame to be displayed on the LED matrix.
     * The frame data is provided as a uint8_t array.
     * @param buffer Pointer to an array of N uint8_t containing the frame data.
     */
    template<size_t N>
    void setFrame(const uint8_t(&buffer)[N]){
        // Copy data in case it is being modified inside prepareFrame().
        size_t frameSize = expectedFrameSize();
        uint8_t data[frameSize];
        // Limit the amount of data to the smaller of N or frameSize
        // to avoid buffer overflows.
        auto length = (N < frameSize) ? N : frameSize;
        memcpy(data, buffer, length);
        prepareFrame(data);
        sendFrame(data, length);
    }

    /**
     * Renders a specific frame from the current sequence.
     * @param frameNumber The index of the frame to render (0-based). 
     *        If the frame number exceeds the total number of frames, it will wrap around using modulo.
     */
    void renderFrame(uint32_t frameNumber){
        _currentFrameNumber = frameNumber % _framesCount;
        renderFrame();
    }

    /**
     * Renders the current frame in the sequence.
     * This method retrieves the frame data for the current frame, prepares it, 
     * and sends it to the LED matrix display. 
     * It also updates the duration for the current frame based on the frame sequence
     */
    void renderFrame(){
        size_t frameSize = expectedFrameSize();
        uint8_t data[frameSize];
        getCurrentFrameData(data);
        setCurrentDuration();
        prepareFrame(data);
        sendFrame(data, frameSize);
    }

    /**
     * Plays the current frame sequence.
     * @param looping If true, the sequence will loop indefinitely until stopped.
     */
    void play(bool looping = false){
        _loop = looping;
        _sequenceDone = false;
        do {
            nextFrame();
            delay(_duration);
            if (_loop && _sequenceDone) {
                _sequenceDone = false;
            }
        } while (_sequenceDone == false);
    }

    /**
     * Gets the duration for the current frame.
     * The duration is extracted from the frame sequence data and is used to determine 
     * how long the current frame should be displayed before advancing to the next frame.
     * @return The duration in milliseconds for the current frame.
     */
    uint32_t getCurrentDuration() const {
        return _duration;
    }

    /**
     * Gets the index of the current frame being displayed.
     * This index is used to track the position within the frame sequence and is updated each time nextFrame() is called.
     * @return The index of the current frame.
     */
    uint32_t getCurrentFrameNumber() const {
        return _currentFrameNumber;
    }

    /**
     * Gets the total number of frames in the current sequence.
     * This is calculated based on the size of the frame sequence data and the expected size of each frame.
     * @return The total number of frames in the current sequence.
     */
    uint32_t getFrameCount() const {
        return _framesCount;
    }

    /**
     * Sets the frame sequence for the LED matrix display using a raw pointer and size.
     * @param frames Pointer to the frame sequence data.
     * @param bytes Total size of the data in bytes.
     * @param is32Bit Whether the data is in 32-bit format (big endian) or 8-bit format.
     */
    void setSequence(const uint8_t* frames, size_t bytes, bool is32Bit = false) {
        _currentFrameNumber = 0;
        _frames = frames;
        size_t frameSize = expectedFrameSize();
        // Calculate frame count based on frame size + interval (4 bytes)
        _framesCount = bytes / (frameSize + 4); 
        _framesAre32Bit = is32Bit;
    }

    /**
     * Sets the frame sequence for the LED matrix display.
     * The frames are provided as a 2D array of 32-bit integers.
     * Each frame consists of N 32-bit integers followed by a duration value.
     * @param frames A reference to a 2D array containing the frame sequence data.
     */
    template<size_t N, size_t M>
    void setSequence(const uint32_t (&frames)[N][M]) {
        setSequence((const uint8_t*)frames, sizeof(frames), true);
    }

    /**
     * Sets the frame sequence for the LED matrix display.
     * The frames are provided as a 2D array of 8-bit integers.
     * Each frame consists of N 8-bit integers followed by a duration value of 4 bytes.
     * The duration bytes are translated to uint32_t internally.
     * @param frames A reference to a 2D array containing the frame sequence data.
     */
    template<size_t N, size_t M>
    void setSequence(const uint8_t (&frames)[N][M]) {
        setSequence((const uint8_t*)frames, sizeof(frames));
    }

    /**
     * Sets the frame sequence for the LED matrix display with explicit used size.
     * This is useful when the provided 2D array is larger than the actual used data.
     * @param frames A reference to a 2D array containing the frame sequence data.
     * @param usedBytes The number of bytes actually used in the buffer.
     */
    template<size_t N, size_t M>
    void setSequence(const uint8_t (&frames)[N][M], size_t usedBytes) {
        setSequence((const uint8_t*)frames, usedBytes);
    }

    /**
     * Sets a callback function to be called when the frame sequence is done.
     * WARNING: callbacks are fired from ISR. The execution time will be limited.
     * Therefore, avoid using delay() or any blocking code inside the callback.
     * @param callBack Pointer to the callback function
     */
    void setSequenceDoneCallback(voidFuncPtr callBack){
        _sequenceDoneCallBack = callBack;
    }

    /**
     * Clears the LED matrix display by sending a frame with all LEDs turned off.
     */
    void clear() {
        const uint32_t fullOff[] = {
            0x00000000,
            0x00000000,
            0x00000000
        };
        setFrame(fullOff);
        #ifdef MATRIX_WITH_ARDUINOGRAPHICS
        memset(_canvasBuffer, 0, sizeof(_canvasBuffer));
        #endif
    }


#ifdef MATRIX_WITH_ARDUINOGRAPHICS
    /**
     * Loads pixel data from a 2D array into a flat frame buffer.
     * Each pixel is represented as a binary value (0 or 1).
     * The output frame buffer is filled with 8-bit integers.
     * @param pixelData A reference to a 2D array containing the pixel data.
     * @param outputFrameBuffer Pointer to an array of uint8_t to hold the frame
     */
    template<size_t N, size_t M>
    void loadPixelsToFrameBuffer(const uint8_t (&pixelData)[N][M], uint8_t* outputFrameBuffer) {
        // We use MONOCHROMATIC_FRAME_SIZE but we should calculate it dynamically based on N*M
        // For 12*8 = 96 bits = 12 bytes = MONOCHROMATIC_FRAME_SIZE.
        memset(outputFrameBuffer, 0, (N * M) / 8);
        const uint8_t* pixelDataFlat = &pixelData[0][0];
        
        size_t index;
        for (size_t i = 0; i < (N * M); i++) {
             if (pixelDataFlat[i]) {
                if (_mode == DisplayMode::MonochromaticHorizontal) {
                    outputFrameBuffer[i / 8] |= (1 << (7 - (i % 8)));
                } else if (_mode == DisplayMode::MonochromaticVertical) {
                    // For vertical mode, we need to transpose to column-major order
                    size_t row = i / M;
                    size_t col = i % M;
                    index = col * N + row;
                    outputFrameBuffer[index / 8] |= (1 << (index % 8));
                } else {
                    return; // Unsupported mode for this function
                }                
            }
        }
    }

    /**
     * Renders the current canvas buffer from ArduinoGraphics
     * to the LED matrix display.
     */
    void renderCanvas(){
        loadPixelsToFrameBuffer(_canvasBuffer, _graphicsFrameHolder);
        setFrame(_graphicsFrameHolder);
    };

    /**
     * Sets a pixel on the canvas buffer.
     * Overrides the ArduinoGraphics set() method. Only supports monochromatic pixels.
     * Any non-zero RGB value is treated as "on".
     * @param x The x-coordinate of the pixel.
     * @param y The y-coordinate of the pixel.
     * @param r The red component (used here to indicate pixel on/off).
     * @param g The green component (not used).
     * @param b The blue component (not used).
     */
    virtual void set(int x, int y, uint8_t r, uint8_t g, uint8_t b) override {
      if (y >= canvasHeight || x >= canvasWidth || y < 0 || x < 0) {
        return;
      }
      // the r parameter is (mis)used to set the character to draw with
      _canvasBuffer[y][x] = (r | g | b) > 0 ? 1 : 0;
    }

    /**
     * Ends the text drawing and renders the canvas to the LED matrix.
     * @param scrollDirection SCROLL_LEFT, SCROLL_RIGHT, etc.
     */
    void endText(int scrollDirection = NO_SCROLL) {
      ArduinoGraphics::endText(scrollDirection);
      renderCanvas();
    }

    /**
     * Displays the drawing or captures it to a buffer when 
     * rendering a animation e.g. using endTextAnimation().
     * Functions such as endText() call this internally.
     */
    void endDraw() override {
        ArduinoGraphics::endDraw();

        if (!captureAnimation) {
            renderCanvas();
            return;
        }

        if (captureAnimationFrameRemainingBytes >= MONOCHROMATIC_ANIMATION_FRAME_SIZE) {
            loadPixelsToFrameBuffer(_canvasBuffer, captureAnimationFrame);
            uint32_t speed = _textScrollSpeed;
            memcpy(captureAnimationFrame + MONOCHROMATIC_FRAME_SIZE, &speed, DURATION_SIZE);
            captureAnimationFrame += MONOCHROMATIC_ANIMATION_FRAME_SIZE;
            captureAnimationFrameRemainingBytes -= MONOCHROMATIC_ANIMATION_FRAME_SIZE;
        }
    }

    /**
     * Captures the text animation to a buffer.
     * @param scrollDirection SCROLL_LEFT, SCROLL_RIGHT, etc.
     * @param frames Buffer to store the animation frames.
     * @param bytesUsed Output variable to store the number of bytes used.
     */
    template <size_t N, size_t M>
    void endTextAnimation(int scrollDirection, uint8_t (&frames)[N][M], uint32_t& bytesUsed) {
      captureAnimationFrame = &frames[0][0];
      captureAnimationFrameRemainingBytes = sizeof(frames);

      captureAnimation = true;
      ArduinoGraphics::textScrollSpeed(0);
      ArduinoGraphics::endText(scrollDirection);
      ArduinoGraphics::textScrollSpeed(_textScrollSpeed);
      captureAnimation = false;
        
      bytesUsed = sizeof(frames) - captureAnimationFrameRemainingBytes;
    }

    /**
     * Sets the text scrolling speed for text animations.
     * @param speed Scrolling speed in milliseconds.
     */
    void textScrollSpeed(unsigned long speed) override {
      ArduinoGraphics::textScrollSpeed(speed);
      _textScrollSpeed = speed;
    }

  private:
    uint8_t _graphicsFrameHolder[MONOCHROMATIC_FRAME_SIZE]; // 12 bytes
    uint8_t* captureAnimationFrame = nullptr; // pointer to next frame to write
    uint32_t captureAnimationFrameRemainingBytes = 0; // bytes remaining in buffer
    bool captureAnimation = false;
    static const byte canvasWidth = 12;
    static const byte canvasHeight = 8;
    uint8_t _canvasBuffer[canvasHeight][canvasWidth] = {{0}};
    unsigned long _textScrollSpeed = 100;
#endif

private:

    /**
     * Sends a frame to the LED matrix display
     * @param data Pointer to a byte array containing the frame data. 
     * The size of the array should match the expected size for the current display mode.
     * @param length Size of the data array
     */
    void sendFrame(uint8_t* data, size_t length) {
        if(!_initialized) return;
        _wire->beginTransmission(_address);
        _wire->write(data, length);
        _wire->endTransmission();
    }

    /**
     * Sets the current duration for the current frame.
     * The duration is extracted from the frame sequence data.
     */
    void setCurrentDuration(){
        size_t frameSize = expectedFrameSize();
        size_t durationSize = sizeof(_duration);
        auto frameOffset = _currentFrameNumber * (frameSize + durationSize);
        auto durationOffset = frameOffset + frameSize;
        memcpy(&_duration, _frames + durationOffset, durationSize);
    }

    /**
     * Loads the current frame data into a byte array.
     * The current frame is determined by the internal frame index.
     * @param data Pointer to a byte array to hold the frame data
     */
    void getCurrentFrameData(uint8_t* data) {
        size_t frameSize = expectedFrameSize();
        size_t durationSize = sizeof(_duration);
        auto frameOffset = _currentFrameNumber * (frameSize + durationSize);

        if (_framesAre32Bit) {
            const uint32_t* target = reinterpret_cast<const uint32_t*>(_frames + frameOffset);
            convert32to8bit(target, frameSize / sizeof(uint32_t), data, frameSize);
        } else {
            memcpy(data, _frames + frameOffset, frameSize);
        }
    }

    /**
     * Sends the current display mode to the LED matrix display.
     * It needs to know the current mode on the device to send the correct
     * amount of data.
     */
    bool sendMode(){
        if(!_initialized) return false;        
        DisplayMode modeOnDevice = readMode();
        size_t bufferSize = modeOnDevice == DisplayMode::Grayscale ? GRAYSCALE_FRAME_SIZE : MONOCHROMATIC_FRAME_SIZE;
        uint8_t buf[bufferSize];
        memset(buf, 0, bufferSize);
        _wire->beginTransmission(_address);

        if(_mode == DisplayMode::Grayscale){
            memcpy(buf, GRAYSCALE_IDENTIFIER, MODE_IDENTIFIER_SIZE);
            _wire->write(buf, sizeof(buf));
        } else {
            memcpy(buf, MONOCHROMATIC_IDENTIFIER, MODE_IDENTIFIER_SIZE);
            _wire->write(buf, sizeof(buf));
        }
        uint8_t result = _wire->endTransmission();
        return result == 0;
    }

    /**
     * Prepares the frame data based on the current display mode.
     * For MonochromaticHorizontal mode, it converts the data to column-major order.
     * @param data Pointer to a byte array containing the frame data to be prepared.
     */
    void prepareFrame(uint8_t* data) {
        // 12 bytes expected for monochromatic modes
        if (_mode == DisplayMode::MonochromaticHorizontal) {
            convertToColumnMajor(data);
        }
    }

    /**
     * Returns the expected frame size in bytes based on the current display mode.
     * @return Expected frame size in bytes (12 for monochromatic modes, 48 for grayscale mode)
     */
    uint8_t expectedFrameSize(){
        return (_mode == DisplayMode::Grayscale) ? GRAYSCALE_FRAME_SIZE : MONOCHROMATIC_FRAME_SIZE;
    }

    /**
     * Reads the current display mode from the LED matrix display.
     * @return The current display mode (MonochromaticVertical or Grayscale)
     */
    DisplayMode readMode(){        
        uint8_t buf[MODE_IDENTIFIER_SIZE + 1]; // +1 for pin strap address
        _wire->requestFrom(_address, sizeof(buf));
        size_t index = 0;
        while (_wire->available()) {
            buf[index++] = _wire->read();
        }
        // Skip the first byte (pin strap address)
        if(memcmp(buf + 1, GRAYSCALE_IDENTIFIER, MODE_IDENTIFIER_SIZE) == 0){
            return DisplayMode::Grayscale;
        } else {
            return DisplayMode::MonochromaticVertical;
        }
    }

    /**
     * Convert 32bit big endian frame data to bytes in
     * little endian format.
     * @param input Pointer to the input frame data
     * @param inputLen Number of uint32_t elements in input
     * @param output Pointer to the output byte array
     * @param outputLen Byte size of the output array
     */
    void convert32to8bit(const uint32_t* input, size_t inputLen, uint8_t* output, size_t outputLen) {
        // If output buffer is smaller than input data, limit conversion
        size_t count = (outputLen / 4 < inputLen) ? (outputLen / 4) : inputLen;
        for (size_t i = 0; i < count; i++) {
            uint32_t val = input[i];
            output[i*4 + 0] = (val >> 24) & 0xFF;
            output[i*4 + 1] = (val >> 16) & 0xFF;
            output[i*4 + 2] = (val >> 8) & 0xFF;
            output[i*4 + 3] = val & 0xFF;
        }
    }

    /**
     * Converts 12 bytes of row-wise data in-place into 12 bytes of column-wise data
     * (8 rows x 12 columns = 96 bits = 12 bytes)
     * @param data Pointer to the output data array (12 bytes)
     */
    void convertToColumnMajor(uint8_t data[12]) {
        uint8_t columnMajorData[12];
        memset(columnMajorData, 0, 12);

        for (int col = 0; col < 12; col++) {
            for (int row = 0; row < 8; row++) {
                int pixelIndex = row * 12 + col;
                int sourceByteIndex = pixelIndex / 8;
                int sourceBitOffset = 7 - (pixelIndex % 8);

                if ((data[sourceByteIndex] >> sourceBitOffset) & 1) {
                    columnMajorData[col] |= (1 << row);
                }
            }
        }
        memcpy(data, columnMajorData, 12);
    }

    bool _framesAre32Bit = false;
    uint32_t _currentFrameNumber = 0;    
    const uint8_t* _frames = nullptr;
    uint32_t _framesCount = 0;
    uint32_t _duration = 0;
    bool _loop = false;
    bool _sequenceDone = false;
    voidFuncPtr _sequenceDoneCallBack = nullptr;
    HardwareI2C* _wire;
    uint8_t _address = DEFAULT_ADDRESS;
    DisplayMode _mode = DisplayMode::MonochromaticHorizontal;
    bool _initialized = false;
};
