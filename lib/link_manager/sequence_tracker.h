#pragma once
#include <cstdint>

class SequenceTracker {
public:
    uint32_t next() { return _seq++; }
    uint32_t current() const { return _seq; }
private:
    uint32_t _seq = 0;
};