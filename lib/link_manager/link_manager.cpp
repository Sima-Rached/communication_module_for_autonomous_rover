#include "link_manager.h"
#include "../protocol/serializer.h"

LinkManager::LinkManager(Transport* transport) : _transport(transport) {}

void LinkManager::begin() {
    _lastReceiveMs = millis(); // don't immediately report "link lost" on boot
}

void LinkManager::update() {
    _hasNewTelemetry = false;
    _hasNewCommand = false;

    uint8_t buffer[64];
    int len = _transport->receive(buffer, sizeof(buffer));
    if (len > 0) {
        _handleIncoming(buffer, len);
    }
}

void LinkManager::_handleIncoming(const uint8_t* buffer, int len) {
    MessageType type = Serializer::peekType(buffer, len);

    switch (type) {
        case MessageType::TELEMETRY: {
            TelemetryMsg msg;
            if (Serializer::decode(buffer, len, msg)) {
                _latestTelemetry = msg;
                _hasNewTelemetry = true;
                _lastReceiveMs = millis();
            }
            break;
        }
        case MessageType::COMMAND: {
            CommandMsg msg;
            if (Serializer::decode(buffer, len, msg)) {
                _latestCommand = msg;
                _hasNewCommand = true;
                _lastReceiveMs = millis();
            }
            break;
        }
        case MessageType::ACK:
            // TODO: match against a pending-ACK table once you need
            // guaranteed delivery rather than best-effort telemetry.
            _lastReceiveMs = millis();
            break;
        default:
            // Unknown/garbled packet — ignore.
            break;
    }
}

bool LinkManager::sendTelemetry(const TelemetryMsg& msgIn) {
    uint32_t now = millis();
    if (now - _lastSendMs < MIN_SEND_INTERVAL_MS) return false; // self-throttled

    TelemetryMsg msg = msgIn;
    msg.seqNum = nextSeqNum();

    uint8_t buffer[64];
    size_t len = Serializer::encode(msg, buffer, sizeof(buffer));
    if (len == 0) return false;

    bool ok = _transport->send(buffer, len);
    if (ok) _lastSendMs = now;
    return ok;
}

bool LinkManager::sendCommand(const CommandMsg& msgIn) {
    // Commands are user/operator-triggered, so they are NOT rate-limited
    // the same way telemetry is — a return-to-base command should go out
    // immediately, not wait for the next throttle window.
    CommandMsg msg = msgIn;
    msg.seqNum = nextSeqNum();

    uint8_t buffer[64];
    size_t len = Serializer::encode(msg, buffer, sizeof(buffer));
    if (len == 0) return false;

    return _transport->send(buffer, len);
}

bool LinkManager::isLinkLost() const {
    return (millis() - _lastReceiveMs) > LINK_TIMEOUT_MS;
}
