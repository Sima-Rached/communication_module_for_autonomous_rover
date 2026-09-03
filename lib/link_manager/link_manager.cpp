#include "link_manager.h"
#include "../protocol/serializer.h"

LinkManager::LinkManager(Transport* transport) : _transport(transport) {}

void LinkManager::begin() {
    _health.begin(); // so that the link isn't considered lost immediately on startup
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
                _health.markReceived();
            }
            break;
        }
        case MessageType::COMMAND: {
            CommandMsg msg;
            if (Serializer::decode(buffer, len, msg)) {
                _latestCommand = msg;
                _hasNewCommand = true;
                _health.markReceived();
            }
            break;
        }
        case MessageType::ACK:
            //later
            //match against a pending-ACK table once switch over
            //method from best-effort to guaranteed delivery.
            _health.markReceived();
            break;
        default:
            //ignore in case unknown message type
            break;
    }
}

template <typename MsgT>
bool LinkManager::_encodeAndSend(MsgT& msg) {
    msg.seqNum = _sequencer.next();

    uint8_t buffer[64];
    size_t len = Serializer::encode(msg, buffer, sizeof(buffer));
    if (len == 0) return false; 
    //if serialzer returns 0,
    // it means the buffer was too small to encode the message

    return _transport->send(buffer, len);
}

bool LinkManager::sendTelemetry(const TelemetryMsg& msgIn) {
    if (!_rateLimiter.allowSend()) return false; // self-throttled

    TelemetryMsg msg = msgIn;
    bool ok = _encodeAndSend(msg);
    if (ok) _rateLimiter.markSent();
    return ok;
}

bool LinkManager::sendCommand(const CommandMsg& msgIn) {
    // Commands are user/operator-triggered, so they are NOT rate-limited
    // and do not wait for the next throttle window in case the command is return to base.
    CommandMsg msg = msgIn;
    return _encodeAndSend(msg);
}