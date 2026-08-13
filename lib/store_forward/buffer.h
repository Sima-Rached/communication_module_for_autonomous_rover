#pragma once
#include "../protocol/message.h"

// ============================================================
// StoreForwardBuffer (COMMS-9)
//
// Simple fixed-size ring buffer of TelemetryMsg, used when the link
// is down (LinkManager::isLinkLost() == true). On reconnect, flush()
// drains it through LinkManager in order.
//
// Deliberately in-RAM only for the prototype (no SD card / flash
// persistence yet) — fine for short field-comms gaps; revisit if you
// need to survive a power cycle while out of range.
// ============================================================

class StoreForwardBuffer {
public:
    static constexpr size_t CAPACITY = 32;

    // Returns false if buffer is full (oldest entries are NOT
    // overwritten silently — caller should know data is being dropped).
    bool push(const TelemetryMsg& msg);

    bool isEmpty() const { return _count == 0; }
    bool isFull() const { return _count == CAPACITY; }
    size_t count() const { return _count; }

    // Pop oldest entry. Returns false if empty.
    bool pop(TelemetryMsg& out);

private:
    TelemetryMsg _entries[CAPACITY];
    size_t _head = 0;   // next write position
    size_t _tail = 0;   // next read position
    size_t _count = 0;
};
