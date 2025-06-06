#pragma once
#include <Arduino.h>

namespace ptz {
class Joystick {
public:
  void begin(uint8_t xp, uint8_t yp);
  int16_t velocity();

private:
  uint8_t xp_ = 34;
  uint8_t yp_ = 35;
};
} // namespace ptz
