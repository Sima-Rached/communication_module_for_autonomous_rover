#pragma once
#include <Arduino.h>
#include "../protocol/message.h"

// ============================================================
// UartBridge — Raspberry Pi <-> ESP32 link
//
// ONLY compiled/used on the rover-side board (NODE_ROLE_ROVER).
// The GCS-side board talks to the PC over USB/serial instead, which
// for a first prototype can just reuse Serial (USB-CDC) directly in
// main.cpp rather than needing its own class — add one here later if
// the GCS-side protocol grows more complex.
//
// Wire format kept intentionally simple for Phase A: same fixed-size
// structs as the radio link, sent raw over UART2 (not the USB-Serial
// used for debug prints, to keep the two streams separate).
// ============================================================

class UartBridge {
public:
    explicit UartBridge(HardwareSerial& serial, uint32_t baud = 115200);

    void begin();

    // Non-blocking: returns true if a full CommandMsg was received
    // from the Pi (e.g. Pi forwarding an operator command that arrived
    // via some other path, or Pi-generated commands).
    bool receiveFromPi(CommandMsg& outMsg);

    // Push telemetry received over the radio link up to the Pi/ROS2.
    bool sendToPi(const TelemetryMsg& msg);

private:
    HardwareSerial& _serial;
    uint32_t _baud;
};
