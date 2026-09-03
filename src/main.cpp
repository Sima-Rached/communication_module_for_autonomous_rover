#include <Arduino.h>
#include "../lib/transport/transport_wifi.h"
#include "../lib/link_manager/link_manager.h"
#include "../lib/store_forward/buffer.h"
#include "../lib/mission/mission_controller.h"

#if defined(NODE_ROLE_ROVER)
#include "../lib/uart_bridge/uart_bridge.h"
#endif

// Wi-Fi credentials for the peer-to-peer AP link (Phase A only).
// Ground Control Station (GCS) board runs the Access Point (AP) with these creds; rover board joins it.

static const char* WIFI_SSID = "rover-proto";
static const char* WIFI_PASS = "roverpass123";
static const uint16_t WIFI_PORT = 4210;

//  Shared objects 
WiFiTransport transport(WIFI_SSID, WIFI_PASS, WIFI_PORT);
LinkManager linkManager(&transport);
StoreForwardBuffer storeForward;

#if defined(NODE_ROLE_ROVER)
UartBridge uartBridge(Serial2); // UART2 reserved for Pi link

// Owns mission state + the autonomous low-battery-return decision.
MissionController missionController(/* lowBatteryThresholdPercent = */ 20);
#endif

uint32_t lastTelemetrySampleMs = 0;
static const uint32_t TELEMETRY_SAMPLE_INTERVAL_MS = 2000;

void setup() {
    Serial.begin(115200);
    delay(500);

#if defined(NODE_ROLE_ROVER)
    Serial.println("[main] Booting as ROVER node");
#elif defined(NODE_ROLE_GCS)
    Serial.println("[main] Booting as GCS node");
#endif

    if (!transport.begin()) {
        Serial.println("[main] Transport init FAILED — halting");
        while (true) delay(1000);
    }

    linkManager.begin();

#if defined(NODE_ROLE_ROVER)
    uartBridge.begin();
#endif

    Serial.println("[main] Setup complete");
}

#if defined(NODE_ROLE_ROVER)
// ROVER role: sample fake telemetry (placeholder for real ROS2/
// Pi data once uartBridge.receiveFromPi-style flow is fleshed
// out further), send it, and forward any GCS command up to the Pi.
void loopRover() {
    // 0. Update mission/status state from the rover's OWN sensor readings. 
    uint8_t currentBatteryPercent = 82; // placeholder 
    bool obstacleDetected = false;      // placeholder 
    missionController.update(currentBatteryPercent, obstacleDetected);

    // 1. Periodically build + send telemetry (placeholder values).
    uint32_t now = millis();
    if (now - lastTelemetrySampleMs >= TELEMETRY_SAMPLE_INTERVAL_MS) {
        lastTelemetrySampleMs = now;

        TelemetryMsg msg;
        msg.timestampMs    = now;
        msg.batteryPercent = currentBatteryPercent;
        msg.posX            = 0.0f; // placeholder
        msg.posY            = 0.0f; // placeholder
        msg.missionState    = missionController.state();
        msg.statusFlags     = missionController.flags();

        bool sent = linkManager.sendTelemetry(msg);
        if (!sent) {
            // Link busy (rate-limited) or down —> buffer it.
            if (!storeForward.push(msg)) {
                Serial.println("[loopRover] Store-forward buffer FULL, dropping sample");
            }
        }
    }

    // 2. Drain store-forward buffer opportunistically once link is up.
    if (!linkManager.isLinkLost() && !storeForward.isEmpty()) {
        TelemetryMsg queued;
        if (storeForward.pop(queued)) {
            if (!linkManager.sendTelemetry(queued)) {
                storeForward.push(queued); // put it back, try again next loop
            }
        }
    }

    // 3. Handle inbound commands from GCS. Routed through
    //    missionController.applyCommand() rather than acted on directly
    linkManager.update();
    if (linkManager.hasNewCommand()) {
        const CommandMsg& cmd = linkManager.latestCommand();
        bool applied = missionController.applyCommand(cmd);
        Serial.print("[loopRover] Command received: ");
        Serial.print(static_cast<int>(cmd.command));
        Serial.println(applied ? " (applied)" : " (REFUSED -- low battery return in progress)");
        uartBridge.sendToPi(TelemetryMsg{}); // placeholder — replace with a
                                              // dedicated "forward command to
                                              // Pi" call once uart_bridge grows
                                              // a matching sendCommandToPi().
    }

    // 4. Comms-loss -> safe state (NFR-SAF-01). Hook this up to your
    //    mobility subsystem's actual safe-state trigger once that
    //    interface exists.
    if (linkManager.isLinkLost()) {
        Serial.println("[loopRover] LINK LOST — safe-state should trigger here");
    }

    // plan b : if link is lost for too long , strategies could be to reconnect 
    //         then maybe to restart then finally to return to base.
}
#endif

#if defined(NODE_ROLE_GCS)
// GCS role: relay any received telemetry to the PC over USB serial 
void loopGcs() {
    linkManager.update();

    if (linkManager.hasNewTelemetry()) {
        const TelemetryMsg& t = linkManager.latestTelemetry();
        Serial.printf("[loopGcs] Telemetry: batt=%d%% pos=(%.2f,%.2f) mission=%d flags=0x%02X rssi=%d\n",
                      t.batteryPercent, t.posX, t.posY,
                      static_cast<int>(t.missionState), t.statusFlags,
                      transport.getSignalStrength());
        // Note: if flags & StatusFlag::LOW_BATTERY, the GCS is only
        // being INFORMED the rover is returning to base
    }

    if (linkManager.isLinkLost()) {
        Serial.println("[loopGcs] LINK LOST — no telemetry received recently");
    }

    // TODO: read operator commands from USB serial and call linkManager.sendCommand(...) here.
}
#endif

void loop() {
#if defined(NODE_ROLE_ROVER)
    loopRover();
#elif defined(NODE_ROLE_GCS)
    loopGcs();
#endif
    delay(20);
}
