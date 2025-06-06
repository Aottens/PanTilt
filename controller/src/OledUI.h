#pragma once
#include "ptz_proto.h"
#include <Adafruit_SSD1306.h>

namespace ptz {
class OledUI {
public:
  void begin();
  void update(const HeadStatus &st, bool zoom);
  Adafruit_SSD1306 &display() { return display_; }

private:
  Adafruit_SSD1306 display_{128, 64, &Wire, -1};
};
} // namespace ptz
