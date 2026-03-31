#pragma once

#include "Modulino.h"

class ModulinoMotors : public Module {
public:
	/**
	 * @brief Supported motor current decay modes.
	 */
	enum class DecayMode : uint8_t {
		SLOW = 0,
		MIXED_30_FAST_70_SLOW = 1,
		MIXED_60_FAST_40_SLOW = 2,
		FAST = 3,
	};

	static constexpr uint8_t MODE_DC = 0;
	static constexpr uint8_t MODE_STEPPER = 1;

	static constexpr int16_t MAX_SPEED = 32767;
	static constexpr uint16_t ADC_FULL_SCALE = 4095;
	static constexpr uint16_t ADC_REF_MV = 3300;
	static constexpr uint16_t ISEN_RESISTOR_OHMS = 4700;
	static constexpr uint16_t KISEN_FULL_SCALE = 7500;
	static constexpr uint16_t KISEN_HALF_SCALE = 3750;

	/**
	 * @brief Construct a Modulino Motors instance.
	 * @param address I2C address, or 0xFF to auto-discover.
	 * @param hubPort Optional hub port selector.
	 * @param stepsPerRevolution Full-step motor steps per shaft revolution.
	 */
	ModulinoMotors(uint8_t address = 0xFF, ModulinoHubPort* hubPort = nullptr, int16_t stepsPerRevolution = -1)
		: Module(address, "MOTORS", hubPort),
			_stepsPerRevolution(stepsPerRevolution) {}

	/**
	 * @brief Construct a Modulino Motors instance using default address and no hub port.
	 * @param stepsPerRevolution Full-step motor steps per shaft revolution.
	 */
	explicit ModulinoMotors(int stepsPerRevolution)
		: Module(0xFF, "MOTORS", nullptr),
			_stepsPerRevolution((stepsPerRevolution >= 1 && stepsPerRevolution <= 32767)
				? static_cast<int16_t>(stepsPerRevolution)
				: static_cast<int16_t>(-1)) {}

	/**
	 * @brief Construct a Modulino Motors instance.
	 * @param hubPort Hub port selector.
	 * @param address I2C address, or 0xFF to auto-discover.
	 * @param stepsPerRevolution Full-step motor steps per shaft revolution.
	 */
	ModulinoMotors(ModulinoHubPort* hubPort, uint8_t address = 0xFF, int16_t stepsPerRevolution = -1)
		: Module(address, "MOTORS", hubPort),
			_stepsPerRevolution(stepsPerRevolution) {}

	/**
	 * @brief Discover the motors module on known addresses.
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

	/**
	 * @brief Stop both DC outputs.
	 * @return True on successful command write.
	 */
	bool stop() {
		return setDcSpeedRaw(0, 0);
	}

	/**
	 * @brief Release stepper coils immediately without changing the default move behavior.
	 * @return True on successful command write.
	 */
	bool release() {
		uint8_t cmd[8] = {CMD_STEPPER, 0, 0, 0, 0, 1, 0, 1};
		return sendCommand(cmd, sizeof(cmd));
	}

	/**
	 * @brief Energize and hold stepper coils immediately without changing default move behavior.
	 * @return True on successful command write.
	 */
	bool hold() {
		uint8_t cmd[8] = {CMD_STEPPER, 0, 0, 0, 0, 1, 0, 0};
		return sendCommand(cmd, sizeof(cmd));
	}

	/**
	 * @brief Set motor A speed as percentage of full scale.
	 * @param percent Speed in range 0..100.
	 * @return True if accepted and command sent.
	 */
	bool setSpeedA(uint8_t percent) {
		if (percent > 100) {
			return false;
		}
		_speedA = percent;
		return updateDcSpeedFromPercent();
	}

	/**
	 * @brief Set motor B speed as percentage of full scale.
	 * @param percent Speed in range 0..100.
	 * @return True if accepted and command sent.
	 */
	bool setSpeedB(uint8_t percent) {
		if (percent > 100) {
			return false;
		}
		_speedB = percent;
		return updateDcSpeedFromPercent();
	}

	/**
	 * @brief Set inversion flag for motor A direction.
	 * @param invert True to invert A direction.
	 * @return True on successful command write.
	 */
	bool setInvertA(bool invert) {
		_invertA = invert;
		return updateDcSpeedFromPercent();
	}

	/**
	 * @brief Set inversion flag for motor B direction.
	 * @param invert True to invert B direction.
	 * @return True on successful command write.
	 */
	bool setInvertB(bool invert) {
		_invertB = invert;
		return updateDcSpeedFromPercent();
	}

	/**
	 * @brief Get configured speed percentage for motor A.
	 * @return Speed percentage in range 0..100.
	 */
	uint8_t speedA() const {
		return _speedA;
	}

	/**
	 * @brief Get configured speed percentage for motor B.
	 * @return Speed percentage in range 0..100.
	 */
	uint8_t speedB() const {
		return _speedB;
	}

	/**
	 * @brief Get inversion state for motor A.
	 * @return True when A direction is inverted.
	 */
	bool invertA() const {
		return _invertA;
	}

	/**
	 * @brief Get inversion state for motor B.
	 * @return True when B direction is inverted.
	 */
	bool invertB() const {
		return _invertB;
	}

	/**
	 * @brief Set raw signed DC speeds for channels A and B.
	 * @param speedA Raw signed speed for A in range -32767..32767.
	 * @param speedB Raw signed speed for B in range -32767..32767.
	 * @return True on successful command write.
	 */
	bool setDcSpeedRaw(int16_t speedA, int16_t speedB) {
		if (speedA < -MAX_SPEED || speedA > MAX_SPEED || speedB < -MAX_SPEED || speedB > MAX_SPEED) {
			return false;
		}

		uint8_t cmd[5];
		cmd[0] = CMD_SPEED_DC;
		cmd[1] = static_cast<uint8_t>(speedA & 0xFF);
		cmd[2] = static_cast<uint8_t>((speedA >> 8) & 0xFF);
		cmd[3] = static_cast<uint8_t>(speedB & 0xFF);
		cmd[4] = static_cast<uint8_t>((speedB >> 8) & 0xFF);
		return sendCommand(cmd, sizeof(cmd));
	}

	/**
	 * @brief Command a stepper move using period-based speed.
	 * @param steps Signed number of steps.
	 * @param speedPeriod Step period in 0.1 ms timer ticks (1..65535).
	 * @return True on successful command write.
	 */
	bool moveStepper(int32_t steps, uint16_t speedPeriod) {
		return moveStepper(steps, speedPeriod, _releaseOnCompleteDefault);
	}

	/**
	 * @brief Command a stepper move using period-based speed.
	 * @param steps Signed number of steps.
	 * @param speedPeriod Step period in 0.1 ms timer ticks (1..65535).
	 * @param releaseOnComplete True to release the coils after the move, false to keep holding torque.
	 * @return True on successful command write.
	 * @note The first step is executed immediately when the command is accepted.
	 *       Remaining steps are paced by speedPeriod.
	 */
	bool moveStepper(int32_t steps, uint16_t speedPeriod, bool releaseOnComplete) {
		if (speedPeriod < 1) {
			return false;
		}

		uint8_t cmd[8];
		cmd[0] = CMD_STEPPER;
		cmd[1] = static_cast<uint8_t>(steps & 0xFF);
		cmd[2] = static_cast<uint8_t>((steps >> 8) & 0xFF);
		cmd[3] = static_cast<uint8_t>((steps >> 16) & 0xFF);
		cmd[4] = static_cast<uint8_t>((steps >> 24) & 0xFF);
		cmd[5] = static_cast<uint8_t>(speedPeriod & 0xFF);
		cmd[6] = static_cast<uint8_t>((speedPeriod >> 8) & 0xFF);
		cmd[7] = static_cast<uint8_t>(releaseOnComplete ? 1 : 0);
		return sendCommand(cmd, sizeof(cmd));
	}

	/**
	 * @brief Command a stepper move using target RPM.
	 * @param steps Signed number of steps.
	 * @param rpm Target shaft speed in RPM.
	 * @return True if inputs are valid and command is sent.
	 */
	bool moveStepperRpm(int32_t steps, float rpm) {
		return moveStepperRpm(steps, rpm, _releaseOnCompleteDefault);
	}

	/**
	 * @brief Command a stepper move using target RPM.
	 * @param steps Signed number of steps.
	 * @param rpm Target shaft speed in RPM.
	 * @param releaseOnComplete True to release the coils after the move, false to keep holding torque.
	 * @return True if inputs are valid and command is sent.
	 */
	bool moveStepperRpm(int32_t steps, float rpm, bool releaseOnComplete) {
		if (_stepsPerRevolution < 1 || rpm <= 0.0f) {
			return false;
		}

		const uint32_t effectiveStepsPerRev = static_cast<uint32_t>(_stepsPerRevolution) * (_halfStepEnabled ? 2UL : 1UL);
		const float pulsesPerMinute = rpm * static_cast<float>(effectiveStepsPerRev);
		if (pulsesPerMinute <= 0.0f) {
			return false;
		}

		const float periodFloat = 60000000.0f / pulsesPerMinute;
		if (periodFloat < 1.0f || periodFloat > 65535.0f) {
			return false;
		}

		const uint16_t periodUs = static_cast<uint16_t>(periodFloat);
		return moveStepper(steps, periodUs, releaseOnComplete);
	}

	/**
	 * @brief Set decay mode using a typed enum.
	 * @param decayMode Desired decay mode.
	 * @return True on successful command write.
	 */
	bool setDecay(DecayMode decayMode) {
		return setDecay(static_cast<uint8_t>(decayMode));
	}

	/**
	 * @brief Set decay mode using raw value.
	 * @param decayMode Raw decay mode in range 0..3.
	 * @return True on successful command write.
	 */
	bool setDecay(uint8_t decayMode) {
		if (decayMode > 3) {
			return false;
		}
		uint8_t cmd[2] = { CMD_DECAY, decayMode };
		const bool ok = sendCommand(cmd, sizeof(cmd));
		if (ok) {
			_decayMode = decayMode;
		}
		return ok;
	}

	/**
	 * @brief Set DC PWM frequency.
	 * @param frequencyHz PWM frequency in Hz (200..60000).
	 * @return True on successful command write.
	 */
	bool setFrequency(uint16_t frequencyHz) {
		if (frequencyHz < 200 || frequencyHz > 60000) {
			return false;
		}
		uint8_t cmd[3];
		cmd[0] = CMD_FREQ_DC;
		cmd[1] = static_cast<uint8_t>(frequencyHz & 0xFF);
		cmd[2] = static_cast<uint8_t>((frequencyHz >> 8) & 0xFF);
		const bool ok = sendCommand(cmd, sizeof(cmd));
		if (ok) {
			_frequencyHz = frequencyHz;
		}
		return ok;
	}

	/**
	 * @brief Configure half-full-scale current sense mode.
	 * @param enabled True to enable half-full-scale mode.
	 * @return True on successful command write.
	 */
	bool setHalfFullScaleEnabled(bool enabled) {
		uint8_t cmd[2] = { CMD_HFS, static_cast<uint8_t>(enabled ? 1 : 0) };
		const bool ok = sendCommand(cmd, sizeof(cmd));
		if (ok) {
			_hfsEnabled = enabled;
		}
		return ok;
	}

	/**
	 * @brief Configure the default release behavior for subsequent stepper moves.
	 * @param enabled True to release coils after the final step, false to hold position.
	 * @return Always true.
	 */
	bool setReleaseOnComplete(bool enabled) {
		_releaseOnCompleteDefault = enabled;
		return true;
	}

	/**
	 * @brief Switch operating mode between DC and stepper.
	 * @param enabled True for stepper mode, false for DC mode.
	 * @return True on successful command write.
	 */
	bool setStepperModeEnabled(bool enabled) {
		uint8_t cmd[2] = { CMD_MODE, static_cast<uint8_t>(enabled ? MODE_STEPPER : MODE_DC) };
		const bool ok = sendCommand(cmd, sizeof(cmd));
		if (ok) {
			_mode = enabled ? MODE_STEPPER : MODE_DC;
		}
		return ok;
	}

	/**
	 * @brief Configure step mode.
	 * @param enabled True for half-step, false for full-step.
	 * @return True on successful command write.
	 */
	bool setHalfStepEnabled(bool enabled) {
		uint8_t cmd[2] = { CMD_STEP_MODE, static_cast<uint8_t>(enabled ? 1 : 0) };
		const bool ok = sendCommand(cmd, sizeof(cmd));
		if (ok) {
			_halfStepEnabled = enabled;
		}
		return ok;
	}

	/**
	 * @brief Read latest telemetry from the module.
	 * @return True on successful read and decode.
	 */
	bool update() {
		uint8_t data[5] = {0};
		if (!read(data, sizeof(data))) {
			return false;
		}

		_senseRawA = static_cast<uint16_t>(data[0] | (data[1] << 8));
		_senseRawB = static_cast<uint16_t>(data[2] | (data[3] << 8));

		const uint8_t flags = data[4];
		_busy = (flags & FLAG_BUSY) != 0;
		_mode = (flags & FLAG_MODE) ? MODE_STEPPER : MODE_DC;
		_halfStepEnabled = (flags & FLAG_STEP_MODE) != 0;
		_hfsEnabled = (flags & FLAG_HFS) != 0;
		_decayMode = (flags & FLAG_DECAY_MASK) >> FLAG_DECAY_SHIFT;
		_releaseOnCompleteReported = (flags & FLAG_RELEASE) != 0;
		return true;
	}

	/**
	 * @brief Get raw current-sense ADC reading for channel A.
	 * @return Raw ADC-derived telemetry count.
	 */
	uint16_t sensedRawA() const {
		return _senseRawA;
	}

	/**
	 * @brief Get raw current-sense ADC reading for channel B.
	 * @return Raw ADC-derived telemetry count.
	 */
	uint16_t sensedRawB() const {
		return _senseRawB;
	}

	/**
	 * @brief Get estimated channel A current in mA.
	 * @return Estimated current in milliamps.
	 */
	float sensedCurrentA() const {
		return senseRawToMa(_senseRawA, _hfsEnabled);
	}

	/**
	 * @brief Get estimated channel B current in mA.
	 * @return Estimated current in milliamps.
	 */
	float sensedCurrentB() const {
		return senseRawToMa(_senseRawB, _hfsEnabled);
	}

	/**
	 * @brief Get busy flag from last telemetry update.
	 * @return True while module reports active move.
	 */
	bool busy() const {
		return _busy;
	}

	/**
	 * @brief Get HFS state from last telemetry update.
	 * @return True if HFS is enabled.
	 */
	bool halfFullScaleEnabled() const {
		return _hfsEnabled;
	}

	/**
	 * @brief Get current operating mode from last telemetry update.
	 * @return True if stepper mode is active.
	 */
	bool stepperModeEnabled() const {
		return _mode == MODE_STEPPER;
	}

	/**
	 * @brief Get step mode from last telemetry update.
	 * @return True if half-step mode is active.
	 */
	bool halfStepEnabled() const {
		return _halfStepEnabled;
	}

	/**
	 * @brief Get the release-on-complete state from last telemetry update.
	 * @return True if the stepper releases its coils after a move.
	 */
	bool releaseOnComplete() const {
		return _releaseOnCompleteReported;
	}

	/**
	 * @brief Get default release behavior used when no per-move flag is provided.
	 * @return True if future moves default to release-on-complete.
	 */
	bool releaseOnCompleteDefault() const {
		return _releaseOnCompleteDefault;
	}

	/**
	 * @brief Get decay mode from last telemetry update.
	 * @return Decay mode as a typed enum.
	 */
	DecayMode decayMode() const {
		return static_cast<DecayMode>(_decayMode);
	}

	/**
	 * @brief Get raw decay mode from last telemetry update.
	 * @return Raw decay mode value in range 0..3.
	 */
	uint8_t decayModeRaw() const {
		return _decayMode;
	}

	/**
	 * @brief Get configured DC PWM frequency.
	 * @return Frequency in Hz.
	 */
	uint16_t frequency() const {
		return _frequencyHz;
	}

	/**
	 * @brief Get configured motor full-step resolution.
	 * @return Full-step count per shaft revolution, or < 1 when unset.
	 */
	int16_t stepsPerRevolution() const {
		return _stepsPerRevolution;
	}

	/**
	 * @brief Set motor full-step resolution.
	 * @param value Full-step count per shaft revolution, must be >= 1.
	 * @return True when accepted.
	 */
	bool setStepsPerRevolution(int16_t value) {
		if (value < 1) {
			return false;
		}
		_stepsPerRevolution = value;
		return true;
	}

private:
	static constexpr uint8_t CMD_MODE = static_cast<uint8_t>('M');
	static constexpr uint8_t CMD_SPEED_DC = static_cast<uint8_t>('S');
	static constexpr uint8_t CMD_STEPPER = static_cast<uint8_t>('G');
	static constexpr uint8_t CMD_DECAY = static_cast<uint8_t>('T');
	static constexpr uint8_t CMD_STEP_MODE = static_cast<uint8_t>('H');
	static constexpr uint8_t CMD_FREQ_DC = static_cast<uint8_t>('F');
	static constexpr uint8_t CMD_HFS = static_cast<uint8_t>('X');

	static constexpr uint8_t FLAG_BUSY = 0x01;
	static constexpr uint8_t FLAG_MODE = 0x02;
	static constexpr uint8_t FLAG_STEP_MODE = 0x04;
	static constexpr uint8_t FLAG_HFS = 0x08;
	static constexpr uint8_t FLAG_DECAY_MASK = 0x30;
	static constexpr uint8_t FLAG_DECAY_SHIFT = 4;
	static constexpr uint8_t FLAG_RELEASE = 0x40;

	/**
	 * @brief Send a command padded to firmware wire length.
	 * @param msg Pointer to command bytes.
	 * @param len Number of bytes in @p msg (max 8).
	 * @return True on successful I2C write.
	 */
	bool sendCommand(const uint8_t* msg, size_t len) {
		if (len > 8) {
			return false;
		}
		uint8_t padded[8] = {0};
		memcpy(padded, msg, len);
		return write(padded, sizeof(padded));
	}

	/**
	 * @brief Recompute and push DC speeds from percentage and invert settings.
	 * @return True on successful command write.
	 */
	bool updateDcSpeedFromPercent() {
		int16_t rawA = static_cast<int16_t>((static_cast<int32_t>(_speedA) * MAX_SPEED) / 100);
		int16_t rawB = static_cast<int16_t>((static_cast<int32_t>(_speedB) * MAX_SPEED) / 100);
		if (_invertA) {
			rawA = static_cast<int16_t>(-rawA);
		}
		if (_invertB) {
			rawB = static_cast<int16_t>(-rawB);
		}
		return setDcSpeedRaw(rawA, rawB);
	}

	/**
	 * @brief Convert raw telemetry count to motor current estimate.
	 * @param raw Raw ADC-derived current-sense value.
	 * @param hfsEnabled Whether half-full-scale mode is active.
	 * @return Estimated current in milliamps.
	 */
	float senseRawToMa(uint16_t raw, bool hfsEnabled) const {
		const float kisen = hfsEnabled ? static_cast<float>(KISEN_HALF_SCALE) : static_cast<float>(KISEN_FULL_SCALE);
		return (static_cast<float>(raw) * static_cast<float>(ADC_REF_MV) * kisen) /
					 (static_cast<float>(ADC_FULL_SCALE) * static_cast<float>(ISEN_RESISTOR_OHMS));
	}

private:
	uint8_t match[1] = { 0x48 };

	uint8_t _speedA = 0;
	bool _invertA = false;
	uint8_t _speedB = 0;
	bool _invertB = false;

	uint16_t _frequencyHz = 20000;
	uint8_t _mode = MODE_DC;
	bool _halfStepEnabled = false;
	uint8_t _decayMode = 0;
	bool _hfsEnabled = false;
	bool _releaseOnCompleteDefault = false;
	bool _releaseOnCompleteReported = false;
	bool _busy = false;

	uint16_t _senseRawA = 0;
	uint16_t _senseRawB = 0;
	int16_t _stepsPerRevolution = -1;
};