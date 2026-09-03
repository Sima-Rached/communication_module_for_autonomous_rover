#pragma once
#include "../protocol/message.h"

// MissionController (rover-side only)

//   1. Tracking current MissionState + StatusFlag condition bits.
//   2. Applying incoming GCS commands to that state.

// The low-battery -> RETURNING_TO_BASE decision is made ENTIRELY
// on the rover, inside update(), using only the rover's own battery
// reading. It is never requested from, confirmed by, or dependent
// on anything arriving over the link.

class MissionController {
public:
    explicit MissionController(uint8_t lowBatteryThresholdPercent = 20)
        : _lowBatteryThreshold(lowBatteryThresholdPercent) {}

    // Call every loop iteration with the rover's own current sensor
    // readings. 

    void update(uint8_t currentBatteryPercent, bool obstacleDetected) {
        if (currentBatteryPercent <= _lowBatteryThreshold) {
            _flags |= StatusFlag::LOW_BATTERY;
            if (_state != MissionState::RETURNING_TO_BASE) {
                _state = MissionState::RETURNING_TO_BASE; // rover's own decision
            }
        } else {
            _flags &= static_cast<uint8_t>(~StatusFlag::LOW_BATTERY); 
            // clear the bit if battery is above threshold
        }

        if (obstacleDetected) {
            _flags |= StatusFlag::AVOIDING_OBSTACLE;
        } else {
            _flags &= static_cast<uint8_t>(~StatusFlag::AVOIDING_OBSTACLE); 
            // clear the bit if no obstacle
        }
    }

    // Call when a CommandMsg arrives from the GCS. 
    // Returns false if the command was refused 

    bool applyCommand(const CommandMsg& cmd) {

        if ((_flags & StatusFlag::LOW_BATTERY)) {
            return false; // refused meaning rover keeps returning to base
        }

        switch (cmd.command) {
            case CommandType::START_MISSION:  _state = MissionState::NAVIGATING; break;
            case CommandType::STOP:           _state = MissionState::STOPPED; break;
            case CommandType::RETURN_TO_BASE: _state = MissionState::RETURNING_TO_BASE; break;
            case CommandType::SET_PARAMS:     break;
        }
        return true;
    }

    MissionState state() const { return _state; }
    uint8_t flags() const { return _flags; }

private:
    MissionState _state = MissionState::IDLE;
    uint8_t      _flags = StatusFlag::NONE;
    uint8_t      _lowBatteryThreshold;
};
