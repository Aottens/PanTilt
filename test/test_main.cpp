#include "ptz_proto.h"
#include "controller/src/Joystick.h"
#include "controller/src/Joystick.cpp"
#include <gtest/gtest.h>
#include <cstring>

using namespace ptz;

int16_t clampPan(int16_t val) { return val < PAN_MIN ? PAN_MIN : (val > PAN_MAX ? PAN_MAX : val); }
int16_t clampTilt(int16_t val){ return val < TILT_MIN ? TILT_MIN : (val > TILT_MAX ? TILT_MAX : val); }

TEST(Limits, Clamp) {
  EXPECT_EQ(clampPan(PAN_MAX + 10), PAN_MAX);
  EXPECT_EQ(clampTilt(TILT_MIN - 10), TILT_MIN);
}

TEST(CRC, Values) {
  EXPECT_NE(HEAD_CMD_CRC, 0);
  EXPECT_NE(HEAD_STATUS_CRC, 0);
}

TEST(CRC, Array) {
  constexpr uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
  constexpr uint16_t crc = crc16_calc(data);
  EXPECT_EQ(crc, 0x2BA1);
}

TEST(CRC, StructConstant) {
  constexpr uint16_t c1 = crc16_struct<HeadCmd>();
  constexpr uint16_t c2 = crc16_struct<HeadStatus>();
  EXPECT_EQ(c1, HEAD_CMD_CRC);
  EXPECT_EQ(c2, HEAD_STATUS_CRC);
}

TEST(CRC, StructRuntime) {
  HeadCmd cmd{42, -42, 50};
  uint8_t bytes[sizeof(cmd)];
  std::memcpy(bytes, &cmd, sizeof(cmd));
  uint16_t crc = crc16_calc(bytes);
  EXPECT_EQ(crc, 0xC91A);
}

TEST(Joystick, Velocity) {
  Joystick joy;
  gAnalogReadValue = 2048;
  EXPECT_EQ(joy.velocity(), 0);
  gAnalogReadValue = 0;
  EXPECT_EQ(joy.velocity(), -100);
  gAnalogReadValue = 4095;
  EXPECT_EQ(joy.velocity(), 99);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
