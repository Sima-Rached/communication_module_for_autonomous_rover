#pragma once
#include <cstdint>

#ifdef UNIT_TEST
uint32_t millis();
#else
#include <Arduino.h>
#endif

class LinkHealthMonitor {
public:
    explicit LinkHealthMonitor(uint32_t timeoutMs) : _timeoutMs(timeoutMs) {}
    void begin() { _lastReceiveMs = millis(); }
    void markReceived() { _lastReceiveMs = millis(); }
    bool isLinkLost() const { return (millis() - _lastReceiveMs) > _timeoutMs; }
private:
    uint32_t _timeoutMs;
    uint32_t _lastReceiveMs = 0;
};