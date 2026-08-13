#pragma once
// Portable: avoids a hard Arduino.h dependency so this interface can
// also be exercised by native/host unit tests (e.g. against a fake
// loopback Transport for link_manager tests). On-device, Arduino.h is
// still pulled in transitively via transport_wifi.h / transport_lora.h.
#include <cstdint>
#include <cstddef>

// ============================================================
// Transport — abstract interface
//
// This is the ONLY layer that changes between Phase A (WiFi)
// and Phase B (LoRa). Everything above this (protocol,
// link_manager, store_forward, uart_bridge) talks only to this
// interface and must never know or care which radio is underneath.
// ============================================================

class Transport {
public:
    virtual ~Transport() = default;

    // One-time setup (join WiFi / init LoRa radio, etc.)
    virtual bool begin() = 0;

    // Send raw bytes. Returns true if handed off successfully
    // (does NOT guarantee delivery — that's link_manager's job).
    virtual bool send(const uint8_t* data, size_t len) = 0;

    // Non-blocking receive. Returns number of bytes read into
    // buffer (0 if nothing available), up to maxLen.
    virtual int receive(uint8_t* buffer, size_t maxLen) = 0;

    // Cheap link-alive check (e.g. WiFi.status(), or "radio responsive").
    // This is NOT the same as link_manager's "peer has gone silent"
    // detection — this only reports whether the local radio itself
    // is up and usable.
    virtual bool isLinkUp() = 0;

    // Approximate signal quality, if the underlying radio exposes one
    // (RSSI for both WiFi and LoRa). Return 0 if not meaningful.
    virtual int getSignalStrength() = 0;
};
