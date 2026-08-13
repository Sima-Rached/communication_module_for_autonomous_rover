#pragma once
#include "../transport/transport.h"
#include "../protocol/message.h"

// millis() is provided by Arduino.h on-device. For native/host unit
// tests, the test file supplies its own millis() and this declaration
// just lets link_manager.cpp link against it either way.
#ifdef UNIT_TEST
uint32_t millis();
#else
#include <Arduino.h>
#endif

// ============================================================
// LinkManager (COMMS-10 and friends)
//
// Owns: outgoing sequence numbers, last-received-message timestamp,
// comms-loss detection, and a self-imposed send-rate limit.
//
// The send-rate limit exists specifically so Phase A (WiFi) testing
// doesn't build habits/protocols that would violate LoRa's EU868 duty
// cycle limits once you swap transports in Phase B. Tune
// MIN_SEND_INTERVAL_MS to whatever your team decides is a realistic
// LoRa cadence (a few seconds between messages, not continuous).
// ============================================================

class LinkManager {
public:
    explicit LinkManager(Transport* transport);

    void begin();

    // Call every loop() iteration.
    void update();

    // Queue a telemetry message for sending (rate-limited internally).
    bool sendTelemetry(const TelemetryMsg& msg);

    // Queue a command message for sending.
    bool sendCommand(const CommandMsg& msg);

    // True if a TelemetryMsg / CommandMsg arrived this update() cycle.
    bool hasNewTelemetry() const { return _hasNewTelemetry; }
    bool hasNewCommand() const { return _hasNewCommand; }
    const TelemetryMsg& latestTelemetry() const { return _latestTelemetry; }
    const CommandMsg& latestCommand() const { return _latestCommand; }

    // NFR-SAF-01: has the peer gone silent beyond the timeout?
    bool isLinkLost() const;

    uint32_t nextSeqNum() { return _outSeqNum++; }

    static constexpr uint32_t MIN_SEND_INTERVAL_MS = 3000;   // self-throttle, LoRa-realistic
    static constexpr uint32_t LINK_TIMEOUT_MS      = 15000;  // NFR-SAF-01 trigger point

private:
    Transport* _transport;
    uint32_t _outSeqNum = 0;
    uint32_t _lastSendMs = 0;
    uint32_t _lastReceiveMs = 0;

    bool _hasNewTelemetry = false;
    bool _hasNewCommand = false;
    TelemetryMsg _latestTelemetry;
    CommandMsg _latestCommand;

    void _handleIncoming(const uint8_t* buffer, int len);
};
