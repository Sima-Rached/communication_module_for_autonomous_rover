#include "buffer.h"

bool StoreForwardBuffer::push(const TelemetryMsg& msg) {
    if (isFull()) return false;
    _entries[_head] = msg;
    _head = (_head + 1) % CAPACITY;
    _count++;
    return true;
}

bool StoreForwardBuffer::pop(TelemetryMsg& out) {
    if (isEmpty()) return false;
    out = _entries[_tail];
    _tail = (_tail + 1) % CAPACITY;
    _count--;
    return true;
}
