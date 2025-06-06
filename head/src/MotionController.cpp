#include "MotionController.h"
#include <Arduino.h>

namespace ptz {

bool MotionController::begin() {
  pan_.setMaxSpeed(1000);
  tilt_.setMaxSpeed(1000);
  pan_.setAcceleration(500);
  tilt_.setAcceleration(500);
  return true;
}

void MotionController::setTarget(int16_t pan, int16_t tilt, int16_t vel_pct) {
  pan = constrain(pan, PAN_MIN, PAN_MAX);
  tilt = constrain(tilt, TILT_MIN, TILT_MAX);
  tgtPan_ = pan;
  tgtTilt_ = tilt;
  float spd = map(vel_pct, 0, 100, 50, 2000);
  pan_.setMaxSpeed(spd);
  tilt_.setMaxSpeed(spd);
  pan_.moveTo(pan * 10); // assume 10 steps per deg
  tilt_.moveTo(tilt * 10);
}

void MotionController::task() {
  pan_.run();
  tilt_.run();

  int16_t encPan = encoderPan_.deg();
  int16_t encTilt = encoderTilt_.deg();
  long errPan = abs(pan_.currentPosition() / 10 - encPan);
  long errTilt = abs(tilt_.currentPosition() / 10 - encTilt);
  if (errPan > 16 || errTilt > 16) {
    if (!errStart_)
      errStart_ = millis();
    if (millis() - errStart_ > 100) {
      pan_.stop();
      tilt_.stop();
      flags_ |= FLAG_ENCODER_ERR;
    }
  } else {
    errStart_ = 0;
  }
}

} // namespace ptz
