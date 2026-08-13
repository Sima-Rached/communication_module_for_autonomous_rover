# Rover Comms Module — PlatformIO Firmware

Phase A (WiFi prototype) skeleton, structured so Phase B (LoRa) is a
transport-layer swap, not a rewrite. See `comms_subsystem_report.pdf`
for the full architecture writeup this project implements.

## Setup

1. Install the **PlatformIO** extension in VS Code.
2. Open this folder (`comms_firmware/`) as the VS Code workspace root.
3. PlatformIO will auto-detect `platformio.ini` and offer to install
   the `espressif32` platform + libraries on first build.

## Project layout

```
comms_firmware/
├── platformio.ini            # board config, two flashable roles: rover / gcs
├── src/
│   └── main.cpp               # setup()/loop(), wires everything together
├── lib/
│   ├── transport/
│   │   ├── transport.h         # abstract interface — THE swap point (WiFi -> LoRa)
│   │   ├── transport_wifi.*    # Phase A: AP + UDP link
│   │   └── transport_lora.*    # Phase B: stub, not yet implemented
│   ├── protocol/
│   │   ├── message.h           # TelemetryMsg / CommandMsg / AckMsg structs
│   │   └── serializer.h        # struct <-> bytes (memcpy-based, fixed size)
│   ├── link_manager/
│   │   └── link_manager.*      # seq numbers, self-throttle, comms-loss detection
│   ├── store_forward/
│   │   └── buffer.*            # ring buffer, used while link is down
│   └── uart_bridge/
│       └── uart_bridge.*       # rover-side only: talks to the Raspberry Pi
└── test/
    └── test_serializer/         # native (no hardware) unit tests
```

## Flashing two boards

This project defines two environments in `platformio.ini`:

```bash
pio run -e rover -t upload   # flash the rover-side board
pio run -e gcs   -t upload   # flash the GCS-side board
```

Each just sets a different build flag (`NODE_ROLE_ROVER` /
`NODE_ROLE_GCS`); it's the same firmware source either way. The
`#if defined(NODE_ROLE_...)` blocks in `main.cpp` and `transport_wifi.cpp`
branch on this to decide AP-vs-station and rover-vs-GCS loop behavior.

Open the Serial Monitor per board (`pio device monitor`) to watch
debug output — `[main]`, `[loopRover]`, `[loopGcs]`, `[WiFiTransport]`
prefixes tell you which layer logged what.

## Running the native unit tests (no hardware needed)

```bash
pio test -e native
```

This runs `test/test_serializer/test_serializer.cpp`, which satisfies
COMMS-4's Definition of Done: encode -> decode round-trips every field
exactly, with zero data loss, and rejects malformed/short buffers.

`link_manager` and `store_forward` are also written to be portable
enough for native testing against a mock `Transport` (see the
`LoopbackTransport` pattern used during development) — worth adding
as a second native test target if you want CI-style coverage before
this leaves prototype stage.

## Phase A -> Phase B swap (WiFi -> LoRa)

In `src/main.cpp`, this line:

```cpp
WiFiTransport transport(WIFI_SSID, WIFI_PASS, WIFI_PORT);
```

becomes, once `transport_lora.cpp` is filled in (see TODOs in
`transport_lora.h`):

```cpp
LoRaTransport transport(868.0);
```

Nothing else in `main.cpp`, `link_manager`, `store_forward`, or
`uart_bridge` needs to change — they only ever call the `Transport`
interface (`send()`, `receive()`, `isLinkUp()`, `getSignalStrength()`),
never anything WiFi-specific directly.

## Known prototype-stage simplifications (fix before field use)

- **UART framing**: `uart_bridge` assumes clean, whole-message writes
  from the Pi with no start/end delimiter. Add proper framing
  (e.g. a `0x7E ... 0x7E` wrapper or COBS encoding) before relying on
  this over a noisy/long UART run.
- **ACK handling**: `AckMsg` is defined and partially wired into
  `link_manager`, but there's no pending-ACK table yet — telemetry is
  currently best-effort, not guaranteed-delivery. Fine for Phase A;
  revisit once you need confirmed command delivery (e.g. for
  return-to-base).
- **Store-forward is RAM-only**: buffered telemetry is lost on power
  cycle. Fine for short field-comms gaps; add flash/SD persistence if
  you need it to survive a reboot while out of range.
- **`loopRover()`'s command-forward-to-Pi step is a placeholder** —
  it currently calls `uartBridge.sendToPi(TelemetryMsg{})` as a stand-in;
  give `UartBridge` a proper `sendCommandToPi(const CommandMsg&)` method
  once the Pi-side listener for commands exists.
