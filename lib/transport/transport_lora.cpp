#include "transport_lora.h"

// STUB IMPLEMENTATION

// Deliberately stubbed out waiting for Phase B (SX1262 radio) to be implemented. 

LoRaTransport::LoRaTransport(float frequencyMHz)
    : _frequencyMHz(frequencyMHz) {}

bool LoRaTransport::begin() {
    Serial.println("[LoRaTransport] NOT YET IMPLEMENTED — Phase B stub.");
    _initialized = false;
    return false;
}

bool LoRaTransport::send(const uint8_t* data, size_t len) {
    (void)data; (void)len;
    return false;
}

int LoRaTransport::receive(uint8_t* buffer, size_t maxLen) {
    (void)buffer; (void)maxLen;
    return 0;
}

bool LoRaTransport::isLinkUp() {
    return false;
}

int LoRaTransport::getSignalStrength() {
    return 0;
}
