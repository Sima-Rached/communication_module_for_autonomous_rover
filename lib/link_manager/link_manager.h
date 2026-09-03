#pragma once
#include "../transport/transport.h"
#include "../protocol/message.h"
#include "sequence_tracker.h"
#include "rate_limiter.h"
#include "link_health_monitor.h"


// LinkManager's single responsibility is
// orchestration: encode a message, ask its collaborators whether/how
// to send it, hand bytes to the Transport, and decode whatever comes
// back.

//collaborators:
//   - SequenceTracker    -> sequencing scheme changes
//   - RateLimiter        -> throttle/duty-cycle policy changes
//   - LinkHealthMonitor   -> loss-detection strategy changes

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
    bool isLinkLost() const { return _health.isLinkLost(); }

    uint32_t nextSeqNum() { return _sequencer.next(); }

    static constexpr uint32_t MIN_SEND_INTERVAL_MS = 3000;   // self-throttle, LoRa-realistic
    static constexpr uint32_t LINK_TIMEOUT_MS      = 15000;  // NFR-SAF-01 trigger point

private:
    Transport* _transport;
    SequenceTracker    _sequencer;
    RateLimiter        _rateLimiter{MIN_SEND_INTERVAL_MS};
    LinkHealthMonitor  _health{LINK_TIMEOUT_MS};

    bool _hasNewTelemetry = false;
    bool _hasNewCommand = false;
    TelemetryMsg _latestTelemetry;
    CommandMsg _latestCommand;

    void _handleIncoming(const uint8_t* buffer, int len);

    // Shared send path for telemetry/command to avoid duplicating
    // the encode+transport->send() sequence in both public methods.
    template <typename MsgT>
    bool _encodeAndSend(MsgT& msg);
};