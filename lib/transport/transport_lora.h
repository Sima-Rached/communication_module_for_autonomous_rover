#pragma once
#include "transport.h"
#include <RadioLib.h>

// ============================================================
// LoRaTransport — Phase B implementation (STUB)
//
// This is intentionally not wired into main.cpp yet. Once you're
// ready for Phase B (COMMS-8 in the backlog), the swap is:
//
//   Transport* radio = new WiFiTransport(...);
//        becomes
//   Transport* radio = new LoRaTransport(...);
//
// ...and nothing else in the codebase changes, because everything
// above the Transport interface only calls send()/receive()/
// isLinkUp()/getSignalStrength().
//
// TODO (Phase B):
//   - Wire SPI pins to match your board (Heltec V3 has the SX1262
//     already wired internally — check board pinout docs, no manual
//     wiring needed if using an integrated board).
//   - Implement begin(): radio.begin(frequency, bandwidth, spreadingFactor, ...)
//   - Implement send(): radio.transmit(data, len)
//   - Implement receive(): radio.receive(buffer, maxLen), non-blocking
//     via radio.startReceive() + interrupt flag, not a blocking call.
//   - Respect EU868 duty cycle limits — do not transmit continuously.
// ============================================================

class LoRaTransport : public Transport {
public:
    LoRaTransport(float frequencyMHz = 868.0);

    bool begin() override;
    bool send(const uint8_t* data, size_t len) override;
    int  receive(uint8_t* buffer, size_t maxLen) override;
    bool isLinkUp() override;
    int  getSignalStrength() override;

private:
    float _frequencyMHz;
    // SX1262 radio(new Module(...));   // Heltec V3 pin mapping — fill in Phase B
    bool _initialized = false;
};
