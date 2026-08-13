#include "uart_bridge.h"
#include "../protocol/serializer.h"

UartBridge::UartBridge(HardwareSerial& serial, uint32_t baud)
    : _serial(serial), _baud(baud) {}

void UartBridge::begin() {
    _serial.begin(_baud);
}

bool UartBridge::receiveFromPi(CommandMsg& outMsg) {
    // Simplest possible framing for the prototype: if at least
    // sizeof(CommandMsg) bytes are waiting, try to decode them.
    // NOTE: this has no start/end delimiter yet, so it assumes
    // clean, whole-message writes from the Pi side. Add a proper
    // framing byte (e.g. 0x7E ... 0x7E) before this leaves prototype
    // stage — noted here rather than silently deferred.
    if (_serial.available() < (int)sizeof(CommandMsg)) return false;

    uint8_t buffer[sizeof(CommandMsg)];
    _serial.readBytes(buffer, sizeof(buffer));
    return Serializer::decode(buffer, sizeof(buffer), outMsg);
}

bool UartBridge::sendToPi(const TelemetryMsg& msg) {
    uint8_t buffer[sizeof(TelemetryMsg)];
    size_t len = Serializer::encode(msg, buffer, sizeof(buffer));
    if (len == 0) return false;
    _serial.write(buffer, len);
    return true;
}
