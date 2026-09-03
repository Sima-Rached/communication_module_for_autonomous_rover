#pragma once
#include "transport.h"
#include <WiFi.h>
#include <WiFiUdp.h>

// WiFiTransport — Phase A implementation

// Uses ESP32 AP mode + UDP, deliberately, rather than a router +
// TCP connection:
//   - AP mode = peer-to-peer, no external infrastructure needed,
//     which mirrors how the eventual LoRa link behaves.
//   - UDP = connectionless, packet-based, matches LoRa's framing
//     model far better than a stateful TCP stream would.


class WiFiTransport : public Transport {
public:
    WiFiTransport(const char* ssid, const char* password, uint16_t port);

    bool begin() override;
    bool send(const uint8_t* data, size_t len) override;
    int  receive(uint8_t* buffer, size_t maxLen) override;
    bool isLinkUp() override;
    int  getSignalStrength() override;

private:
    const char* _ssid;
    const char* _password;
    uint16_t _port;
    WiFiUDP _udp;

    IPAddress _peerIP;  // learned once the first packet arrives
    bool _peerKnown = false;
};

// Fixed AP address the ESP32 uses by default in AP mode (192.168.4.1).
// The rover-side (station) board sends its first packet to this address
// to "introduce" itself
// the GCS then replies to whatever source IP that packet arrived from.
static const IPAddress GCS_AP_IP(192, 168, 4, 1);
