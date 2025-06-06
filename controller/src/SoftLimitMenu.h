#pragma once
#include "ptz_proto.h"
#include <Preferences.h>
#include <Adafruit_SSD1306.h>
#include "Joystick.h"

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
  void edit(Joystick &joy, Adafruit_SSD1306 &disp);
  Limits limits() const { return limits_; }

private:
  Preferences prefs_;
  Limits limits_{PAN_MIN, PAN_MAX, TILT_MIN, TILT_MAX};
};
} // namespace ptz
