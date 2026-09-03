#include "transport_wifi.h"

WiFiTransport::WiFiTransport(const char* ssid, const char* password, uint16_t port)
    : _ssid(ssid), _password(password), _port(port) {}

bool WiFiTransport::begin() {
#if defined(NODE_ROLE_GCS)
    // GCS board = access point. Rover connects to this.
    WiFi.mode(WIFI_AP);
    bool ok = WiFi.softAP(_ssid, _password);
    if (!ok) return false;
    Serial.print("[WiFiTransport] AP started, IP: ");
    Serial.println(WiFi.softAPIP());

#elif defined(NODE_ROLE_ROVER)
    // Rover board = station, connects to the GCS's AP.
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);

    Serial.print("[WiFiTransport] Connecting to AP");
    uint32_t startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 15000) {
        delay(300);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WiFiTransport] Failed to connect to AP");
        return false;
    }
    Serial.print("[WiFiTransport] Connected, IP: ");
    Serial.println(WiFi.localIP());
    _peerIP = GCS_AP_IP;
    _peerKnown = true;
#else
    #error "Define NODE_ROLE_ROVER or NODE_ROLE_GCS in platformio.ini build_flags"
#endif

    return _udp.begin(_port);
}

bool WiFiTransport::send(const uint8_t* data, size_t len) {
    if (!_peerKnown) {
        // GCS hasn't heard from the rover yet so nothing to send to.
        return false;
    }
    _udp.beginPacket(_peerIP, _port);
    _udp.write(data, len);
    return _udp.endPacket() == 1;
}

int WiFiTransport::receive(uint8_t* buffer, size_t maxLen) {
    int packetSize = _udp.parsePacket();
    if (packetSize <= 0) return 0;

    // Learn/refresh the peer's address from whatever just arrived.
    // On the GCS side this is how we discover the rover's IP the
    // first time it speaks
    // on the rover side it just reconfirms the (already known) AP address.
    _peerIP = _udp.remoteIP();
    _peerKnown = true;

    int len = _udp.read(buffer, maxLen);
    return len > 0 ? len : 0;
}

bool WiFiTransport::isLinkUp() {
#if defined(NODE_ROLE_GCS)
    return WiFi.softAPgetStationNum() > 0;
#else
    return WiFi.status() == WL_CONNECTED;
#endif
}

int WiFiTransport::getSignalStrength() {
#if defined(NODE_ROLE_ROVER)
    return WiFi.RSSI();
#else
    return 0; // AP side doesn't get a simple single RSSI reading
#endif
}
