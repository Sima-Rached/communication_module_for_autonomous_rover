// Native test run with pio test -e native

// Proves the security-motivated property from mission_controller.h:
// once the rover autonomously decides to return to base on low
// battery, an incoming GCS command cannot redirect it away from that.

#include <unity.h>
#include "../../lib/protocol/message.h"
#include "../../lib/mission/mission_controller.h"

void test_low_battery_triggers_autonomous_return() {
    MissionController mc(/* lowBatteryThresholdPercent = */ 20);
    mc.update(/* battery = */ 15, /* obstacle = */ false);

    TEST_ASSERT_EQUAL(static_cast<int>(MissionState::RETURNING_TO_BASE),
                       static_cast<int>(mc.state()));
    TEST_ASSERT_TRUE(mc.flags() & StatusFlag::LOW_BATTERY);
}

void test_gcs_cannot_redirect_away_from_low_battery_return() {
    MissionController mc(20);
    mc.update(15, false); // battery low -> autonomous return begins

    CommandMsg maliciousOrStaleCommand;
    maliciousOrStaleCommand.command = CommandType::START_MISSION;

    bool applied = mc.applyCommand(maliciousOrStaleCommand);

    TEST_ASSERT_FALSE(applied); // refused
    TEST_ASSERT_EQUAL(static_cast<int>(MissionState::RETURNING_TO_BASE),
                       static_cast<int>(mc.state())); // state unchanged
}

void test_gcs_stop_is_still_allowed_during_low_battery_return() {
    MissionController mc(20);
    mc.update(15, false);

    CommandMsg stopCmd;
    stopCmd.command = CommandType::STOP;

    bool applied = mc.applyCommand(stopCmd);

    TEST_ASSERT_TRUE(applied); // STOP is the deliberate exception
    TEST_ASSERT_EQUAL(static_cast<int>(MissionState::STOPPED),
                       static_cast<int>(mc.state()));
}

void test_gcs_command_applies_normally_once_battery_recovers() {
    MissionController mc(20);
    mc.update(15, false); // low battery -> returning
    mc.update(90, false); // battery recovered -> flag clears

    TEST_ASSERT_FALSE(mc.flags() & StatusFlag::LOW_BATTERY);

    CommandMsg startCmd;
    startCmd.command = CommandType::START_MISSION;
    bool applied = mc.applyCommand(startCmd);

    TEST_ASSERT_TRUE(applied);
    TEST_ASSERT_EQUAL(static_cast<int>(MissionState::NAVIGATING),
                       static_cast<int>(mc.state()));
}

void test_obstacle_flag_is_independent_of_mission_state() {
    MissionController mc(20);
    mc.update(90, true); // healthy battery, obstacle present

    TEST_ASSERT_TRUE(mc.flags() & StatusFlag::AVOIDING_OBSTACLE);
    TEST_ASSERT_FALSE(mc.flags() & StatusFlag::LOW_BATTERY);
    // mission state is whatever it already was (IDLE by default here)
    TEST_ASSERT_EQUAL(static_cast<int>(MissionState::IDLE),
                       static_cast<int>(mc.state()));
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_low_battery_triggers_autonomous_return);
    RUN_TEST(test_gcs_cannot_redirect_away_from_low_battery_return);
    RUN_TEST(test_gcs_stop_is_still_allowed_during_low_battery_return);
    RUN_TEST(test_gcs_command_applies_normally_once_battery_recovers);
    RUN_TEST(test_obstacle_flag_is_independent_of_mission_state);
    return UNITY_END();
}
