#include "Joystick.h"

namespace ptz {

void Joystick::begin(uint8_t xp, uint8_t yp) {
  xp_ = xp;
  yp_ = yp;
  pinMode(xp_, INPUT);
  pinMode(yp_, INPUT);
}

int16_t Joystick::velocity() {
  int x = analogRead(xp_);
  int centered = x - 2048;
  float norm = centered / 2048.0f;
  if (abs(norm) < 0.02f)
    return 0;
  float expo = norm * norm * norm; // exponential response
  int vel = expo * 100;
  vel = constrain(vel, -100, 100);
  return vel;
}

} // namespace ptz
