#ifndef PLAYBACK_ENGINE_H
#define PLAYBACK_ENGINE_H

#include "Config.h"
#include "KeyframeManager.h"
#include "MotorController.h"

// Playback modes for keyframe sequences.
enum class PlaybackMode {
  AB,       // keyframe 0 -> 1
  ABA,      // 0 -> 1 -> 0
  SEQUENCE  // 0 -> 1 -> 2 -> 3 -> loop
};

// PlaybackEngine coordinates motion between keyframes.
class PlaybackEngine {
 public:
  PlaybackEngine();

  void begin(MotorController* mc, KeyframeManager* kf);

  void start(PlaybackMode mode = PlaybackMode::SEQUENCE);
  void stop();
  bool isPlaying() const { return _playing; }

  // Call periodically to advance sequences.
  void update();

 private:
  MotorController* _motor = nullptr;
  KeyframeManager* _keyframes = nullptr;
  PlaybackMode _mode = PlaybackMode::SEQUENCE;
  bool _playing = false;
  int8_t _current = 0;
  int8_t _direction = 1; // for ping-pong

  void queueNext();
};

#endif // PLAYBACK_ENGINE_H
