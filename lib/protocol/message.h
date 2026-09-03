#pragma once
#include <cstdint>
#include <cstddef>

// Message schema (COMMS-2, COMMS-3)

// Kept deliberately small and fixed-size
// this matters even in Phase A, 
// because we self-throttle to LoRa-realistic payload
// sizes/rates from day one.

// SCHEMA_VERSION lets you evolve the format later without silently
// misinterpreting old messages after a firmware update on only one side.

static const uint8_t SCHEMA_VERSION = 1;

enum class MessageType : uint8_t {
    TELEMETRY = 0x01,
    COMMAND   = 0x02,
    ACK       = 0x03,
};

// MissionState: what the rover is currently DOING (mission-level,
// mutually exclusive ). 
// This replaces the old flat RoverStatus enum, which was
// a mix of state + condition bits.
enum class MissionState : uint8_t {
    IDLE              = 0,
    NAVIGATING        = 1,
    RETURNING_TO_BASE = 2,
    STOPPED           = 3,
    FAULT             = 4,
};

// StatusFlag: independent, possibly-simultaneous CONDITIONS the
// rover can be in, orthogonal to MissionState. (Bitmask)
namespace StatusFlag {
    constexpr uint8_t NONE              = 0;
    constexpr uint8_t AVOIDING_OBSTACLE = 1 << 0;
    constexpr uint8_t LOW_BATTERY       = 1 << 1;
    // Room for up to 6 more flags (uint8_t)
}

enum class CommandType : uint8_t {
    START_MISSION  = 0,
    STOP           = 1,
    RETURN_TO_BASE = 2,
    SET_PARAMS     = 3,
};

#pragma pack(push, 1)   // no padding 

struct TelemetryMsg {
    uint8_t      schemaVersion = SCHEMA_VERSION;
    MessageType  type = MessageType::TELEMETRY;
    uint32_t     seqNum;      // for ACK matching + loss detection
    uint32_t     timestampMs;
    uint8_t      batteryPercent; 
    float        posX;
    float        posY;
    MissionState missionState;   
    uint8_t      statusFlags; 
};

struct CommandMsg {
    uint8_t     schemaVersion = SCHEMA_VERSION;
    MessageType type = MessageType::COMMAND;
    uint32_t    seqNum;
    CommandType command;
    float       param1;
    float       param2;
};

struct AckMsg {
    uint8_t     schemaVersion = SCHEMA_VERSION;
    MessageType type = MessageType::ACK;
    uint32_t    ackedSeqNum;
};

#pragma pack(pop)
