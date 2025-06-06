#pragma once
#include <Wire.h>

namespace ptz {

class ZoomDetect {
public:
  bool zoomPresent();
};

} // namespace ptz
