#pragma once
#include <cstdint>

#ifdef UNIT_TEST
uint32_t millis();
#else
#include <Arduino.h>
#endif

class RateLimiter {
public:
    explicit RateLimiter(uint32_t minIntervalMs) : _minIntervalMs(minIntervalMs) {}
    bool allowSend() const { return (millis() - _lastSendMs) >= _minIntervalMs; }
    void markSent() { _lastSendMs = millis(); }
private:
    uint32_t _minIntervalMs;
    uint32_t _lastSendMs = 0;
};