#include "SoftLimitMenu.h"
#include "ptz_proto.h"
#include <Arduino.h>

namespace ptz {

void SoftLimitMenu::begin() {
  prefs_.begin("limits", false);
  limits_.panMin = prefs_.getShort("panMin", PAN_MIN);
  limits_.panMax = prefs_.getShort("panMax", PAN_MAX);
  limits_.tiltMin = prefs_.getShort("tiltMin", TILT_MIN);
  limits_.tiltMax = prefs_.getShort("tiltMax", TILT_MAX);
}

void SoftLimitMenu::edit() {
  prefs_.putShort("panMin", limits_.panMin);
  prefs_.putShort("panMax", limits_.panMax);
  prefs_.putShort("tiltMin", limits_.tiltMin);
  prefs_.putShort("tiltMax", limits_.tiltMax);
}

} // namespace ptz
