#include "ptz_proto.h"
#include <gtest/gtest.h>

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

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
