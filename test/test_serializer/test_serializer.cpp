// ============================================================
// Native test (no ESP32 hardware needed) — run with:
//   pio test -e native
//
// Satisfies COMMS-4's DoD: "encode -> decode -> matches original,
// zero data loss."
// ============================================================
#include <unity.h>
#include <cstring>
#include "../../lib/protocol/message.h"
#include "../../lib/protocol/serializer.h"

void test_telemetry_roundtrip() {
    TelemetryMsg original;
    original.seqNum = 42;
    original.timestampMs = 123456;
    original.batteryPercent = 77;
    original.posX = 12.34f;
    original.posY = -5.6f;
    original.status = RoverStatus::NAVIGATING;

    uint8_t buffer[64];
    size_t len = Serializer::encode(original, buffer, sizeof(buffer));
    TEST_ASSERT_GREATER_THAN(0, len);

    TelemetryMsg decoded;
    bool ok = Serializer::decode(buffer, len, decoded);
    TEST_ASSERT_TRUE(ok);

    TEST_ASSERT_EQUAL_UINT32(original.seqNum, decoded.seqNum);
    TEST_ASSERT_EQUAL_UINT32(original.timestampMs, decoded.timestampMs);
    TEST_ASSERT_EQUAL_UINT8(original.batteryPercent, decoded.batteryPercent);
    TEST_ASSERT_EQUAL_FLOAT(original.posX, decoded.posX);
    TEST_ASSERT_EQUAL_FLOAT(original.posY, decoded.posY);
    TEST_ASSERT_EQUAL(static_cast<int>(original.status), static_cast<int>(decoded.status));
}

void test_command_roundtrip() {
    CommandMsg original;
    original.seqNum = 7;
    original.command = CommandType::RETURN_TO_BASE;
    original.param1 = 0.0f;
    original.param2 = 0.0f;

    uint8_t buffer[64];
    size_t len = Serializer::encode(original, buffer, sizeof(buffer));
    TEST_ASSERT_GREATER_THAN(0, len);

    CommandMsg decoded;
    bool ok = Serializer::decode(buffer, len, decoded);
    TEST_ASSERT_TRUE(ok);
    TEST_ASSERT_EQUAL(static_cast<int>(original.command), static_cast<int>(decoded.command));
}

void test_peek_type() {
    TelemetryMsg t;
    uint8_t buffer[64];
    size_t len = Serializer::encode(t, buffer, sizeof(buffer));
    TEST_ASSERT_EQUAL(static_cast<int>(MessageType::TELEMETRY),
                       static_cast<int>(Serializer::peekType(buffer, len)));
}

void test_decode_rejects_short_buffer() {
    TelemetryMsg decoded;
    uint8_t tinyBuffer[2] = {1, 1};
    bool ok = Serializer::decode(tinyBuffer, sizeof(tinyBuffer), decoded);
    TEST_ASSERT_FALSE(ok);
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_telemetry_roundtrip);
    RUN_TEST(test_command_roundtrip);
    RUN_TEST(test_peek_type);
    RUN_TEST(test_decode_rejects_short_buffer);
    return UNITY_END();
}
