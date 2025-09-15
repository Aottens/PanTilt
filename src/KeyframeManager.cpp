#include "KeyframeManager.h"

KeyframeManager::KeyframeManager() {}

void KeyframeManager::begin() {
  _prefs.begin("keyframes", false);
  for (uint8_t i = 0; i < MAX_KEYFRAMES; ++i) {
    String key = String("kf") + i;
    _cache[i].pan = _prefs.getInt((key + "p").c_str(), 0);
    _cache[i].tilt = _prefs.getInt((key + "t").c_str(), 0);
    _cache[i].zoom = _prefs.getInt((key + "z").c_str(), 0);
  }
}

bool KeyframeManager::store(uint8_t idx, const Keyframe& kf) {
  if (idx >= MAX_KEYFRAMES) return false;
  String key = String("kf") + idx;
  bool ok = true;
  ok &= _prefs.putInt((key + "p").c_str(), kf.pan);
  ok &= _prefs.putInt((key + "t").c_str(), kf.tilt);
  ok &= _prefs.putInt((key + "z").c_str(), kf.zoom);
  if (ok) _cache[idx] = kf;
  return ok;
}

bool KeyframeManager::load(uint8_t idx, Keyframe& out) const {
  if (idx >= MAX_KEYFRAMES) return false;
  out = _cache[idx];
  return true;
}

bool KeyframeManager::capture(uint8_t idx, int32_t pan, int32_t tilt, int32_t zoom) {
  Keyframe kf{pan, tilt, zoom};
  bool ok = store(idx, kf);
#ifdef DEBUG
  if (ok) {
    Serial.printf("Keyframe %u stored: P=%ld T=%ld Z=%ld\n", idx, pan, tilt, zoom);
  }
#endif
  return ok;
}
