#pragma once
#include "ptz_proto.h"
#include <Preferences.h>

namespace ptz {
struct Limits {
  int16_t panMin;
  int16_t panMax;
  int16_t tiltMin;
  int16_t tiltMax;
};

class SoftLimitMenu {
public:
  void begin();
  void edit();
  Limits limits() const { return limits_; }

private:
  Preferences prefs_;
  Limits limits_{PAN_MIN, PAN_MAX, TILT_MIN, TILT_MAX};
};
} // namespace ptz
