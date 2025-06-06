#pragma once
#include "Encoder.h"
#include "ptz_proto.h"
#include <AccelStepper.h>

namespace ptz {

class MotionController {
public:
  bool begin();
  void setTarget(int16_t pan, int16_t tilt, int16_t vel_pct);
  void task();
  uint8_t flags() const { return flags_; }
  int16_t panDeg() const { return encoderPan_.deg(); }
  int16_t tiltDeg() const { return encoderTilt_.deg(); }
  int16_t tgtPan() const { return tgtPan_; }
  int16_t tgtTilt() const { return tgtTilt_; }

private:
  AccelStepper pan_{AccelStepper::DRIVER, 32, 33};
  AccelStepper tilt_{AccelStepper::DRIVER, 26, 27};
  Encoder encoderPan_{0x36};
  Encoder encoderTilt_{0x37};
  int16_t tgtPan_ = 0;
  int16_t tgtTilt_ = 0;
  uint8_t flags_ = 0;
  unsigned long errStart_ = 0;
};

} // namespace ptz
