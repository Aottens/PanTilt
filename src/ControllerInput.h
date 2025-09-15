#ifndef CONTROLLER_INPUT_H
#define CONTROLLER_INPUT_H

#include <Bluepad32.h>
#include "Config.h"
#include "MotorController.h"
#include "KeyframeManager.h"
#include "PlaybackEngine.h"

// ControllerInput handles Nintendo Switch Pro Controller events and maps
// them to PTZ actions.
class ControllerInput {
 public:
  ControllerInput();

  void begin(MotorController* mc, KeyframeManager* kf, PlaybackEngine* pe);

  // Call periodically to process controller input.
  void update();

 private:
  MotorController* _motor = nullptr;
  KeyframeManager* _keyframes = nullptr;
  PlaybackEngine* _playback = nullptr;
  GamepadPtr _gamepad;

  uint16_t _prevButtons = 0;
  uint8_t _nextCapture = 0; // next keyframe slot to store

  // Convert joystick value (-512..512) to speed in steps/s.
  int32_t axisToSpeed(int32_t value);
};

#endif // CONTROLLER_INPUT_H
