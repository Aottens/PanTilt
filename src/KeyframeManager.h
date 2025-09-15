#ifndef KEYFRAME_MANAGER_H
#define KEYFRAME_MANAGER_H

#include <Preferences.h>
#include "Config.h"

// Struct representing a keyframe of pan, tilt and zoom positions.
struct Keyframe {
  int32_t pan;
  int32_t tilt;
  int32_t zoom;
};

// KeyframeManager handles persistent storage of keyframes using NVS.
class KeyframeManager {
 public:
  KeyframeManager();

  // Initialize preferences and load stored keyframes.
  void begin();

  // Store keyframe in slot [0..MAX_KEYFRAMES-1]. Returns false on error.
  bool store(uint8_t idx, const Keyframe& kf);

  // Retrieve keyframe. Returns false if not available.
  bool load(uint8_t idx, Keyframe& out) const;

  // Capture keyframe from current positions.
  bool capture(uint8_t idx, int32_t pan, int32_t tilt, int32_t zoom);

 private:
  Preferences _prefs;
  Keyframe _cache[MAX_KEYFRAMES];
};

#endif // KEYFRAME_MANAGER_H
