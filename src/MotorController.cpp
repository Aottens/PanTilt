#include "MotorController.h"

MotorController::MotorController() {
  _pan.stepPin = PAN_STEP_PIN;
  _pan.dirPin = PAN_DIR_PIN;
  _pan.muxChannel = PAN_ENCODER_CH;
  _pan.stepsPerDegree = PAN_STEPS_PER_DEG;
  _pan.minLimit = PAN_MIN;
  _pan.maxLimit = PAN_MAX;

  _tilt.stepPin = TILT_STEP_PIN;
  _tilt.dirPin = TILT_DIR_PIN;
  _tilt.muxChannel = TILT_ENCODER_CH;
  _tilt.stepsPerDegree = TILT_STEPS_PER_DEG;
  _tilt.minLimit = TILT_MIN;
  _tilt.maxLimit = TILT_MAX;

  _zoom.stepPin = ZOOM_STEP_PIN;
  _zoom.dirPin = ZOOM_DIR_PIN;
  _zoom.muxChannel = ZOOM_ENCODER_CH;
  _zoom.stepsPerDegree = ZOOM_STEPS_PER_MM; // Not degrees but mm
  _zoom.minLimit = ZOOM_MIN;
  _zoom.maxLimit = ZOOM_MAX;
}

bool MotorController::begin() {
  _engine.init();
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  _pan.stepper = _engine.stepperConnectToPin(_pan.stepPin);
  _tilt.stepper = _engine.stepperConnectToPin(_tilt.stepPin);
  _zoom.stepper = _engine.stepperConnectToPin(_zoom.stepPin);

  if (!_pan.stepper || !_tilt.stepper || !_zoom.stepper) {
#ifdef DEBUG
    Serial.println("Stepper allocation failed");
#endif
    return false;
  }

  // Configure steppers
  _pan.stepper->setDirectionPin(_pan.dirPin);
  _tilt.stepper->setDirectionPin(_tilt.dirPin);
  _zoom.stepper->setDirectionPin(_zoom.dirPin);

  _pan.stepper->setEnablePin(DRIVER_ENABLE_PIN);
  _tilt.stepper->setEnablePin(DRIVER_ENABLE_PIN);
  _zoom.stepper->setEnablePin(DRIVER_ENABLE_PIN);

  _pan.stepper->setAutoEnable(false);
  _tilt.stepper->setAutoEnable(false);
  _zoom.stepper->setAutoEnable(false);

  enableMotors(false);

  return true;
}

void MotorController::enableMotors(bool enable) {
  if (enable) {
    digitalWrite(DRIVER_ENABLE_PIN, LOW); // TMC2225 enable low
  } else {
    digitalWrite(DRIVER_ENABLE_PIN, HIGH);
  }
}

void MotorController::homeAll() {
#ifdef DEBUG
  Serial.println("Homing axes using AS5600 encoders...");
#endif
  enableMotors(true);

  // Read each encoder and set current position accordingly
  uint16_t rawPan = readEncoder(_pan.muxChannel);
  uint16_t rawTilt = readEncoder(_tilt.muxChannel);
  uint16_t rawZoom = readEncoder(_zoom.muxChannel);

  _pan.stepper->setCurrentPosition(encoderToSteps(rawPan, _pan));
  _tilt.stepper->setCurrentPosition(encoderToSteps(rawTilt, _tilt));
  _zoom.stepper->setCurrentPosition(encoderToSteps(rawZoom, _zoom));

#ifdef DEBUG
  Serial.println("Homing complete");
#endif
}

void MotorController::moveTo(int32_t pan, int32_t tilt, int32_t zoom,
                             uint32_t speed, uint32_t accel) {
  // Clamp to limits
  pan = constrain(pan, _pan.minLimit, _pan.maxLimit);
  tilt = constrain(tilt, _tilt.minLimit, _tilt.maxLimit);
  zoom = constrain(zoom, _zoom.minLimit, _zoom.maxLimit);

  // Calculate deltas
  int32_t dPan = abs(pan - _pan.stepper->getCurrentPosition());
  int32_t dTilt = abs(tilt - _tilt.stepper->getCurrentPosition());
  int32_t dZoom = abs(zoom - _zoom.stepper->getCurrentPosition());

  int32_t maxDelta = max(dPan, max(dTilt, dZoom));
  if (maxDelta == 0) return;

  // Time to reach target based on default speed
  float time = (float)maxDelta / speed;

  // Configure each axis speed so all finish together
  if (dPan > 0) {
    _pan.stepper->setSpeedInHz(dPan / time);
    _pan.stepper->setAcceleration(accel);
    _pan.stepper->moveTo(pan);
  }
  if (dTilt > 0) {
    _tilt.stepper->setSpeedInHz(dTilt / time);
    _tilt.stepper->setAcceleration(accel);
    _tilt.stepper->moveTo(tilt);
  }
  if (dZoom > 0) {
    _zoom.stepper->setSpeedInHz(dZoom / time);
    _zoom.stepper->setAcceleration(accel);
    _zoom.stepper->moveTo(zoom);
  }
}

void MotorController::jog(int32_t panSpeed, int32_t tiltSpeed, int32_t zoomSpeed) {
  if (panSpeed != 0) {
    _pan.stepper->setSpeedInHz(abs(panSpeed));
    if (panSpeed > 0)
      _pan.stepper->runForward();
    else
      _pan.stepper->runBackward();
  } else {
    _pan.stepper->stopMove();
  }

  if (tiltSpeed != 0) {
    _tilt.stepper->setSpeedInHz(abs(tiltSpeed));
    if (tiltSpeed > 0)
      _tilt.stepper->runForward();
    else
      _tilt.stepper->runBackward();
  } else {
    _tilt.stepper->stopMove();
  }

  if (zoomSpeed != 0) {
    _zoom.stepper->setSpeedInHz(abs(zoomSpeed));
    if (zoomSpeed > 0)
      _zoom.stepper->runForward();
    else
      _zoom.stepper->runBackward();
  } else {
    _zoom.stepper->stopMove();
  }
}

void MotorController::update() {
  // Placeholder for future tasks. FastAccelStepper runs independently.
}

int32_t MotorController::panPosition() const {
  return _pan.stepper->getCurrentPosition();
}

int32_t MotorController::tiltPosition() const {
  return _tilt.stepper->getCurrentPosition();
}

int32_t MotorController::zoomPosition() const {
  return _zoom.stepper->getCurrentPosition();
}

bool MotorController::isBusy() const {
  return _pan.stepper->isRunning() || _tilt.stepper->isRunning() ||
         _zoom.stepper->isRunning();
}

uint16_t MotorController::readEncoder(uint8_t muxChannel) {
  selectMuxChannel(muxChannel);
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(0x0E); // RAW ANGLE register
  Wire.endTransmission(false);
  Wire.requestFrom(AS5600_ADDR, (uint8_t)2);
  uint16_t high = Wire.read();
  uint16_t low = Wire.read();
  return (high << 8) | low;
}

int32_t MotorController::encoderToSteps(uint16_t raw, const Axis& axis) {
  float angle = (raw & 0x0FFF) * 360.0f / 4096.0f; // degrees
  return (int32_t)(angle * axis.stepsPerDegree);
}

void MotorController::selectMuxChannel(uint8_t channel) {
  Wire.beginTransmission(I2C_MUX_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}
