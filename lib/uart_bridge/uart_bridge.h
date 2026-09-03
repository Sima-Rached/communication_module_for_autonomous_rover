#pragma once
#include <Arduino.h>
#include "../protocol/message.h"

// UartBridge — Raspberry Pi <-> ESP32 link

// Wire format kept intentionally simple for Phase A: 
// same fixed-size structs as the radio link, sent raw over UART2

class UartBridge {
public:
    explicit UartBridge(HardwareSerial& serial, uint32_t baud = 115200);

    void begin();

    // Non-blocking: 
    // returns true if a full CommandMsg was received from Pi
    bool receiveFromPi(CommandMsg& outMsg);

    // Push telemetry received over the radio link up to the Pi/ROS2.
    bool sendToPi(const TelemetryMsg& msg);

private:
    HardwareSerial& _serial;
    uint32_t _baud;
};
