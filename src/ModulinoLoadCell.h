#pragma once

#include "Modulino.h"

/**
 * @brief Modulino Load Cell: weight from one or two strain gauge load cells.
 *
 * Channel A (terminal J5) reads with gain 128, channel B (terminal J6) with gain 32.
 * The module excites the bridge at 3.0 V and converts the signal to grams itself,
 * using an offset (tare) and a scale (counts per gram) per channel. Both can be
 * stored in the module, so a calibrated module keeps its calibration on any host.
 */
class ModulinoLoadCell : public Module {
public:
	static constexpr uint8_t CHANNEL_A = 0;
	static constexpr uint8_t CHANNEL_B = 1;

	/** @brief Channels sampled by the module. */
	enum class Mode : uint8_t {
		A = 0,        ///< channel A only (default)
		B = 1,        ///< channel B only
		A_AND_B = 2,  ///< both, alternated: each channel gets about 1 sample/s at 10 SPS, 7.7 at 80 SPS
	};

	/** @brief Converter output data rate. */
	enum class Rate : uint8_t {
		SPS_10 = 0,   ///< 10 samples per second (default), lowest noise, rejects 50/60 Hz mains pickup
		SPS_80 = 1,   ///< 80 samples per second, no mains rejection
	};

	/** @brief Outcome of the last command sent to the module. */
	enum class Result : uint8_t {
		OK = 0,
		UNKNOWN_COMMAND = 1,
		INVALID_ARGUMENT = 2,
		BUSY = 3,              ///< a tare or calibration is still running
		CHANNEL_INACTIVE = 4,  ///< the channel is not sampled in the current mode, or the module is powered down
		SATURATED = 5,         ///< the input is out of range: check the wiring
		SIGNAL_TOO_SMALL = 6,  ///< calibrate(): the known weight is too light for this cell
		TIMEOUT = 7,           ///< the converter stopped delivering samples
		STORAGE_ERROR = 8,     ///< saving to the module's flash failed
		NO_RESPONSE = 0xFF,    ///< the module did not answer
	};

	/**
	 * @brief Construct a Modulino Load Cell instance.
	 * @param address I2C address, or 0xFF to find the module at its default address (0x34).
	 * @param hubPort Optional hub port selector.
	 */
	ModulinoLoadCell(uint8_t address = 0xFF, ModulinoHubPort* hubPort = nullptr)
		: Module(address, "LOADCELL", hubPort) {}

	/**
	 * @brief Construct a Modulino Load Cell instance.
	 * @param hubPort Hub port selector.
	 * @param address I2C address, or 0xFF to find the module at its default address.
	 */
	ModulinoLoadCell(ModulinoHubPort* hubPort, uint8_t address = 0xFF)
		: Module(address, "LOADCELL", hubPort) {}

	/**
	 * @brief Find the module, check that its firmware speaks this library's protocol and
	 * bring it to its power-on state: stored calibration (if any), channel A, 10 SPS.
	 * The first sample follows about 0.5 s later.
	 * @return False if the module is missing or runs an incompatible firmware (update it with
	 * the FirmwareUpdater example).
	 */
	bool begin() {
		if (!Module::begin()) {
			return false;
		}
		uint8_t info[INFO_LEN];
		if (!readPage(PAGE_INFO, info, INFO_LEN) || info[1] != PROTOCOL_VERSION) {
			return false;
		}
		_fwVersion[0] = info[2];
		_fwVersion[1] = info[3];
		_fwVersion[2] = info[4];
		// The module stays powered when the board is reset or reprogrammed: start from its
		// power-on state, not from the mode, rate or unsaved tare of the previous sketch.
		uint8_t cmd[FRAME_LEN] = { CMD_RESET };
		if (sendCommand(cmd, COMMAND_TIMEOUT_MS) != Result::OK) {
			return false;
		}
		_seen[CHANNEL_A] = _count[CHANNEL_A];
		_seen[CHANNEL_B] = _count[CHANNEL_B];
		return true;
	}

	/**
	 * @brief Discover the module on its default address.
	 * @return Detected address encoding, or 0xFF if not found.
	 */
	uint8_t discover() override {
		for (unsigned int i = 0; i < sizeof(match) / sizeof(match[0]); i++) {
			if (scan(match[i])) {
				return match[i];
			}
		}
		return 0xFF;
	}

	// ------------------------------------------------------------------ reading

	/**
	 * @brief Read the latest measurement of both channels from the module.
	 * @return True if the module answered.
	 */
	bool update() {
		uint8_t p[MEAS_LEN];
		if (!readPage(PAGE_MEASUREMENT, p, MEAS_LEN)) {
			return false;
		}
		_status = p[1];
		_cmdCount = p[2];
		_cmdResult = p[3];
		for (uint8_t ch = 0; ch < 2; ch++) {
			const uint8_t* b = &p[ch == CHANNEL_A ? 4 : 16];
			memcpy(&_raw[ch], &b[0], 4);
			memcpy(&_weightMg[ch], &b[4], 4);
			memcpy(&_count[ch], &b[8], 2);
			_flags[ch] = b[10];
		}
		return true;
	}

	/**
	 * @brief Read the module and tell whether a new sample arrived on a channel since the last call.
	 * @param channel CHANNEL_A or CHANNEL_B.
	 */
	bool available(uint8_t channel = CHANNEL_A) {
		uint8_t ch = channel & 1;
		if (!update() || _count[ch] == _seen[ch]) {
			return false;
		}
		_seen[ch] = _count[ch];
		return true;
	}

	/**
	 * @brief Weight in grams from the last update(), filtered and tared.
	 * @return NAN if no scale is set on the channel (see setLoadCell() and calibrate()).
	 */
	float getWeight(uint8_t channel = CHANNEL_A) const {
		int32_t mg = _weightMg[channel & 1];
		return (mg == WEIGHT_INVALID) ? NAN : (float)mg / 1000.0f;
	}

	/** @brief Last raw sample, signed 24-bit converter counts. */
	int32_t getRaw(uint8_t channel = CHANNEL_A) const {
		return _raw[channel & 1];
	}

	/** @brief Samples taken on the channel since power-up (wraps at 65535). */
	uint16_t getSampleCount(uint8_t channel = CHANNEL_A) const {
		return _count[channel & 1];
	}

	/** @brief A scale is set, so getWeight() returns grams. */
	bool isWeightValid(uint8_t channel = CHANNEL_A) const {
		return (_flags[channel & 1] & FLAG_WEIGHT_VALID) != 0;
	}

	/** @brief The load is above the rated capacity set with setLoadCell() or setCapacity(). */
	bool isOverCapacity(uint8_t channel = CHANNEL_A) const {
		return (_flags[channel & 1] & FLAG_OVER_CAPACITY) != 0;
	}

	/** @brief The input is outside the converter's range: cell disconnected or miswired. */
	bool isSaturated(uint8_t channel = CHANNEL_A) const {
		return (_flags[channel & 1] & FLAG_SATURATED) != 0;
	}

	/** @brief A calibration for this channel is stored in the module. */
	bool hasStoredCalibration(uint8_t channel = CHANNEL_A) const {
		return (_flags[channel & 1] & FLAG_STORED) != 0;
	}

	// ------------------------------------------------------------------ calibration

	/**
	 * @brief Describe the load cell from its datasheet: the module then reads grams right away.
	 * Accuracy is the cell's sensitivity tolerance (often 10 to 20 %) until calibrate() is used.
	 * @param mVperV Rated output in mV/V (for example 1.0 or 2.0).
	 * @param capacityGrams Rated capacity in grams.
	 * @param channel CHANNEL_A or CHANNEL_B.
	 */
	bool setLoadCell(float mVperV, uint32_t capacityGrams, uint8_t channel = CHANNEL_A) {
		if (mVperV <= 0.0f || capacityGrams == 0) {
			return false;
		}
		// Ratiometric: full scale is +/-(AVDD/2)/gain = 2^23 counts and the cell gives
		// mVperV * AVDD at rated load, so the excitation voltage cancels out.
		float gain = ((channel & 1) == CHANNEL_A) ? 128.0f : 32.0f;
		float countsPerGram = mVperV * 2.0f * gain * 8388608.0f / 1000.0f / (float)capacityGrams;
		return setScale(countsPerGram, channel) && setCapacity(capacityGrams, channel);
	}

	/**
	 * @brief Set the current load as zero. Keep the cell still: the module averages `samples` samples.
	 * The channel must be sampled in the current mode (setMode()), as for calibrate().
	 * @return True on success, see getLastResult() otherwise.
	 */
	bool tare(uint8_t channel = CHANNEL_A, uint8_t samples = 10) {
		uint8_t cmd[FRAME_LEN] = { CMD_TARE, (uint8_t)(channel & 1), samples };
		return sendCommand(cmd, averagingTimeout(samples)) == Result::OK;
	}

	/**
	 * @brief Compute the scale from a known weight placed on the cell after tare().
	 * Use a weight of at least 10 % of the cell's capacity.
	 * @return True on success, see getLastResult() otherwise.
	 */
	bool calibrate(float knownGrams, uint8_t channel = CHANNEL_A, uint8_t samples = 10) {
		int32_t mg = (int32_t)lroundf(knownGrams * 1000.0f);
		uint8_t cmd[FRAME_LEN] = { CMD_CALIBRATE, (uint8_t)(channel & 1), samples };
		memcpy(&cmd[4], &mg, 4);
		return sendCommand(cmd, averagingTimeout(samples)) == Result::OK;
	}

	/** @brief Store offset, scale, capacity and filter of both channels in the module. */
	bool saveCalibration() {
		uint8_t cmd[FRAME_LEN] = { CMD_STORE, STORE_SAVE };
		return sendCommand(cmd, COMMAND_TIMEOUT_MS) == Result::OK;
	}

	/** @brief Erase the stored calibration. The values in use stay until the next begin() or power-up. */
	bool eraseCalibration() {
		uint8_t cmd[FRAME_LEN] = { CMD_STORE, STORE_ERASE };
		return sendCommand(cmd, COMMAND_TIMEOUT_MS) == Result::OK;
	}

	/** @brief Low-pass filter on the weight: time constant in ms, 0 = off (default). */
	bool setFilter(uint16_t ms, uint8_t channel = CHANNEL_A) {
		uint8_t cmd[FRAME_LEN] = { CMD_FILTER, (uint8_t)(channel & 1), (uint8_t)ms, (uint8_t)(ms >> 8) };
		return sendCommand(cmd, COMMAND_TIMEOUT_MS) == Result::OK;
	}

	/** @brief Raw counts at zero load (restore a calibration kept on the host). */
	bool setOffset(int32_t counts, uint8_t channel = CHANNEL_A) {
		return sendValue(CMD_OFFSET, channel, &counts);
	}

	/** @brief Counts per gram; 0 disables the weight. Negative when the signal wires are swapped. */
	bool setScale(float countsPerGram, uint8_t channel = CHANNEL_A) {
		return sendValue(CMD_SCALE, channel, &countsPerGram);
	}

	/** @brief Rated capacity in grams for isOverCapacity(); 0 disables the check. */
	bool setCapacity(uint32_t grams, uint8_t channel = CHANNEL_A) {
		return sendValue(CMD_CAPACITY, channel, &grams);
	}

	/**
	 * @brief Read offset, scale, capacity and filter of a channel from the module,
	 * for getOffset(), getScale(), getCapacity() and getFilter().
	 */
	bool readSettings(uint8_t channel = CHANNEL_A) {
		uint8_t ch = channel & 1;
		uint8_t p[SETTINGS_LEN];
		if (!readPage(ch == CHANNEL_A ? PAGE_SETTINGS_A : PAGE_SETTINGS_B, p, SETTINGS_LEN)) {
			return false;
		}
		memcpy(&_filterMs[ch], &p[2], 2);
		memcpy(&_offset[ch], &p[4], 4);
		memcpy(&_scale[ch], &p[8], 4);
		memcpy(&_capacity[ch], &p[12], 4);
		return true;
	}

	int32_t getOffset(uint8_t channel = CHANNEL_A) const { return _offset[channel & 1]; }
	float getScale(uint8_t channel = CHANNEL_A) const { return _scale[channel & 1]; }
	uint32_t getCapacity(uint8_t channel = CHANNEL_A) const { return _capacity[channel & 1]; }
	uint16_t getFilter(uint8_t channel = CHANNEL_A) const { return _filterMs[channel & 1]; }

	// ------------------------------------------------------------------ configuration

	/** @brief Select the channels to sample. */
	bool setMode(Mode mode) {
		uint8_t cmd[FRAME_LEN] = { CMD_MODE, (uint8_t)mode };
		return sendCommand(cmd, COMMAND_TIMEOUT_MS) == Result::OK;
	}

	Mode getMode() const {
		return (Mode)((_status >> 4) & 0x3);
	}

	/** @brief Select the output data rate. */
	bool setRate(Rate rate) {
		uint8_t cmd[FRAME_LEN] = { CMD_RATE, (uint8_t)rate };
		return sendCommand(cmd, COMMAND_TIMEOUT_MS) == Result::OK;
	}

	Rate getRate() const {
		return (_status & STATUS_RATE_80) ? Rate::SPS_80 : Rate::SPS_10;
	}

	/** @brief Switch off the converter and the bridge excitation. */
	bool powerDown() {
		uint8_t cmd[FRAME_LEN] = { CMD_POWER, 0 };
		return sendCommand(cmd, COMMAND_TIMEOUT_MS) == Result::OK;
	}

	/** @brief Switch them back on. The first sample arrives about 0.5 s later at 10 SPS. */
	bool powerUp() {
		uint8_t cmd[FRAME_LEN] = { CMD_POWER, 1 };
		return sendCommand(cmd, COMMAND_TIMEOUT_MS) == Result::OK;
	}

	bool isPowered() const {
		return (_status & STATUS_POWERED) != 0;
	}

	/** @brief The converter delivered no sample for 1 s and was restarted. */
	bool hasTimeout() const {
		return (_status & STATUS_TIMEOUT) != 0;
	}

	/** @brief Outcome of the last command (tare(), calibrate(), setMode(), ...). */
	Result getLastResult() const {
		return _lastResult;
	}

	/** @brief Firmware version of the module, for example "0.0.1". */
	String getFirmwareVersion() const {
		return String(_fwVersion[0]) + "." + String(_fwVersion[1]) + "." + String(_fwVersion[2]);
	}

private:
	// Wire protocol, see docs/protocol.md of the module firmware
	static constexpr uint8_t PROTOCOL_VERSION = 1;
	static constexpr uint8_t FRAME_LEN = 8;
	static constexpr uint8_t MEAS_LEN = 28;
	static constexpr uint8_t INFO_LEN = 8;
	static constexpr uint8_t SETTINGS_LEN = 16;
	static constexpr uint8_t PAGE_MEASUREMENT = 0;
	static constexpr uint8_t PAGE_INFO = 1;
	static constexpr uint8_t PAGE_SETTINGS_A = 2;
	static constexpr uint8_t PAGE_SETTINGS_B = 3;
	static constexpr uint8_t CMD_MODE = 'M';
	static constexpr uint8_t CMD_RATE = 'R';
	static constexpr uint8_t CMD_POWER = 'P';
	static constexpr uint8_t CMD_TARE = 'T';
	static constexpr uint8_t CMD_CALIBRATE = 'K';
	static constexpr uint8_t CMD_OFFSET = 'O';
	static constexpr uint8_t CMD_SCALE = 'S';
	static constexpr uint8_t CMD_CAPACITY = 'L';
	static constexpr uint8_t CMD_FILTER = 'F';
	static constexpr uint8_t CMD_STORE = 'W';
	static constexpr uint8_t CMD_RESET = 'X';
	static constexpr uint8_t CMD_PAGE = 'G';
	static constexpr uint8_t STORE_ERASE = 0;
	static constexpr uint8_t STORE_SAVE = 1;
	static constexpr uint8_t STATUS_POWERED = 1 << 0;
	static constexpr uint8_t STATUS_RATE_80 = 1 << 1;
	static constexpr uint8_t STATUS_TIMEOUT = 1 << 2;
	static constexpr uint8_t FLAG_WEIGHT_VALID = 1 << 0;
	static constexpr uint8_t FLAG_OVER_CAPACITY = 1 << 1;
	static constexpr uint8_t FLAG_SATURATED = 1 << 2;
	static constexpr uint8_t FLAG_STORED = 1 << 3;
	static constexpr int32_t WEIGHT_INVALID = INT32_MIN;
	static constexpr uint32_t COMMAND_TIMEOUT_MS = 500;   // a flash write takes about 35 ms

	/**
	 * @brief Read one page. A page other than the measurement is selected first and
	 * returned by the next read only; retry if the module had not processed the selection yet.
	 */
	bool readPage(uint8_t page, uint8_t* buf, uint8_t len) {
		for (uint8_t attempt = 0; attempt < 5; attempt++) {
			if (page != PAGE_MEASUREMENT) {
				uint8_t cmd[FRAME_LEN] = { CMD_PAGE, page };
				write(cmd, FRAME_LEN);
				delay(1);
			}
			if (!read(buf, len)) {
				return false;
			}
			if (buf[0] == page) {
				return true;
			}
		}
		return false;
	}

	/**
	 * @brief Send a command and wait until the module reports it completed
	 * (its command counter moves).
	 */
	Result sendCommand(uint8_t* cmd, uint32_t timeoutMs) {
		_lastResult = Result::NO_RESPONSE;
		if (!update()) {
			return _lastResult;
		}
		uint8_t before = _cmdCount;
		write(cmd, FRAME_LEN);
		uint32_t start = millis();
		while (millis() - start < timeoutMs) {
			delay(2);
			if (update() && _cmdCount != before) {
				_lastResult = (Result)_cmdResult;
				break;
			}
		}
		return _lastResult;
	}

	bool sendValue(uint8_t id, uint8_t channel, const void* value) {
		uint8_t cmd[FRAME_LEN] = { id, (uint8_t)(channel & 1) };
		memcpy(&cmd[4], value, 4);
		return sendCommand(cmd, COMMAND_TIMEOUT_MS) == Result::OK;
	}

	/** @brief Time to average `samples` samples in the current mode and rate, with margin. */
	uint32_t averagingTimeout(uint8_t samples) const {
		uint32_t n = samples ? samples : 10;
		bool both = getMode() == Mode::A_AND_B;
		bool fast = getRate() == Rate::SPS_80;
		// Measured per-channel periods: 100 / 12.5 ms on one channel, 1040 / 130 ms on both
		// (the converter settles after every channel switch).
		uint32_t periodMs = both ? (fast ? 130 : 1040) : (fast ? 13 : 100);
		return n * periodMs * 2 + 1500;
	}

	int32_t _raw[2] = { 0, 0 };
	int32_t _weightMg[2] = { WEIGHT_INVALID, WEIGHT_INVALID };
	uint16_t _count[2] = { 0, 0 };
	uint16_t _seen[2] = { 0, 0 };
	uint8_t _flags[2] = { 0, 0 };
	int32_t _offset[2] = { 0, 0 };
	float _scale[2] = { 0.0f, 0.0f };
	uint32_t _capacity[2] = { 0, 0 };
	uint16_t _filterMs[2] = { 0, 0 };
	uint8_t _status = 0;
	uint8_t _cmdCount = 0;
	uint8_t _cmdResult = 0;
	uint8_t _fwVersion[3] = { 0, 0, 0 };
	Result _lastResult = Result::OK;

protected:
	uint8_t match[1] = { 0x68 };  // same as fw protocol.h (PROTO_NODE_ID)
};
