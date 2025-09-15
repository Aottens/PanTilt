#include "PlaybackEngine.h"

PlaybackEngine::PlaybackEngine() {}

void PlaybackEngine::begin(MotorController* mc, KeyframeManager* kf) {
  _motor = mc;
  _keyframes = kf;
}

void PlaybackEngine::start(PlaybackMode mode) {
  if (!_motor || !_keyframes) return;
  _mode = mode;
  _playing = true;
  _current = 0;
  _direction = 1;
  queueNext();
#ifdef DEBUG
  Serial.println("Playback started");
#endif
}

void PlaybackEngine::stop() {
  _playing = false;
#ifdef DEBUG
  Serial.println("Playback stopped");
#endif
}

void PlaybackEngine::update() {
  if (!_playing) return;
  if (_motor->isBusy()) return;
  queueNext();
}

void PlaybackEngine::queueNext() {
  if (!_playing) return;

  int8_t next = _current + _direction;
  switch (_mode) {
    case PlaybackMode::AB:
      if (next > 1) { stop(); return; }
      break;
    case PlaybackMode::ABA:
      if (next > 1) { _direction = -1; next = 0; }
      break;
    case PlaybackMode::SEQUENCE:
    default:
      if (next >= MAX_KEYFRAMES || !_keyframes) next = 0;
      break;
  }

  Keyframe kf;
  if (_keyframes->load(next, kf)) {
    _motor->moveTo(kf.pan, kf.tilt, kf.zoom);
    _current = next;
  } else {
    // If keyframe not defined, stop playback
    stop();
  }
}
