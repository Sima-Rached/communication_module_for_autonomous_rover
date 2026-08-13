#pragma once
// Portable: only <cstdint> needed, so this header (and serializer.h,
// which only depends on this) can compile both on-device (Arduino
// framework) and in the native/host unit test environment.
#include <cstdint>
#include <cstddef>

// ============================================================
// Message schema (COMMS-2, COMMS-3)
//
// Kept deliberately small and fixed-size — this matters even in
// Phase A, because we self-throttle to LoRa-realistic payload
// sizes/rates from day one (see notes in link_manager.h).
//
// SCHEMA_VERSION lets you evolve the format later without silently
// misinterpreting old messages after a firmware update on only one side.
// ============================================================

static const uint8_t SCHEMA_VERSION = 1;

enum class MessageType : uint8_t {
    TELEMETRY = 0x01,
    COMMAND   = 0x02,
    ACK       = 0x03,
};

enum class RoverStatus : uint8_t {
    IDLE        = 0,
    NAVIGATING  = 1,
    RETURNING   = 2,
    FAULT       = 3,
    LOW_BATTERY = 4,
};

enum class CommandType : uint8_t {
    START_MISSION  = 0,
    STOP           = 1,
    RETURN_TO_BASE = 2,
    SET_PARAMS     = 3,
};

#pragma pack(push, 1)   // no padding — keep wire format compact & predictable

struct TelemetryMsg {
    uint8_t     schemaVersion = SCHEMA_VERSION;
    MessageType type = MessageType::TELEMETRY;
    uint32_t    seqNum;         // for ACK matching + loss detection
    uint32_t    timestampMs;
    uint8_t     batteryPercent; // 0-100
    float       posX;
    float       posY;
    RoverStatus status;
};

struct CommandMsg {
    uint8_t     schemaVersion = SCHEMA_VERSION;
    MessageType type = MessageType::COMMAND;
    uint32_t    seqNum;
    CommandType command;
    float       param1;  // meaning depends on `command` (e.g. mission time budget)
    float       param2;
};

struct AckMsg {
    uint8_t     schemaVersion = SCHEMA_VERSION;
    MessageType type = MessageType::ACK;
    uint32_t    ackedSeqNum;
};

#pragma pack(pop)
