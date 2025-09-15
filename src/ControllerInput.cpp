#include "ControllerInput.h"

ControllerInput::ControllerInput() {}

void ControllerInput::begin(MotorController* mc, KeyframeManager* kf,
                            PlaybackEngine* pe) {
  _motor = mc;
  _keyframes = kf;
  _playback = pe;
  Bluepad32.setup(nullptr, nullptr, nullptr, nullptr);
  Bluepad32.enableNewBluetoothConnections();
}

int32_t ControllerInput::axisToSpeed(int32_t value) {
  // Map -512..512 to -MAX_JOG_SPEED..MAX_JOG_SPEED with deadband
  const int dead = 20;
  if (abs(value) < dead) return 0;
  return (value * (int32_t)MAX_JOG_SPEED) / 512;
}

void ControllerInput::update() {
  Bluepad32.update();
  GamepadPtr gp = Bluepad32.getGamepads()[0];
  if (gp && gp->isConnected()) {
    _gamepad = gp;
  } else {
    _gamepad = nullptr;
    return;
  }

  // Analog joystick handling
  int32_t panSpeed = axisToSpeed(_gamepad->axisX());
  int32_t tiltSpeed = -axisToSpeed(_gamepad->axisY()); // invert Y
  int32_t zoomSpeed = -axisToSpeed(_gamepad->axisRY());
  _motor->jog(panSpeed, tiltSpeed, zoomSpeed);

  uint16_t buttons = _gamepad->buttons();

  auto wasPressed = [&](uint16_t mask) {
    return (buttons & mask) && !(_prevButtons & mask);
  };

  if (wasPressed(BLUEPAD32_BUTTON_ZR)) {
    // Capture current position
    _keyframes->capture(_nextCapture, _motor->panPosition(),
                        _motor->tiltPosition(), _motor->zoomPosition());
    _nextCapture = (_nextCapture + 1) % MAX_KEYFRAMES;
  }

  if (wasPressed(BLUEPAD32_BUTTON_A)) {
    Keyframe k; if (_keyframes->load(0, k))
      _motor->moveTo(k.pan, k.tilt, k.zoom);
  }
  if (wasPressed(BLUEPAD32_BUTTON_B)) {
    Keyframe k; if (_keyframes->load(1, k))
      _motor->moveTo(k.pan, k.tilt, k.zoom);
  }
  if (wasPressed(BLUEPAD32_BUTTON_X)) {
    Keyframe k; if (_keyframes->load(2, k))
      _motor->moveTo(k.pan, k.tilt, k.zoom);
  }
  if (wasPressed(BLUEPAD32_BUTTON_Y)) {
    Keyframe k; if (_keyframes->load(3, k))
      _motor->moveTo(k.pan, k.tilt, k.zoom);
  }

  if (wasPressed(BLUEPAD32_BUTTON_START)) {
    _playback->start();
  }
  if (wasPressed(BLUEPAD32_BUTTON_SELECT)) {
    _playback->stop();
  }

  _prevButtons = buttons;
}
