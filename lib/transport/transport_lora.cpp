#include "transport_lora.h"

// ============================================================
// STUB IMPLEMENTATION
//
// Deliberately non-functional placeholders so the project compiles
// cleanly today, and so the file/class shape is already in place
// when you start Phase B. Fill these in per the TODOs in the header
// once LoRa hardware is on the bench (COMMS-7 → COMMS-8).
// ============================================================

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
