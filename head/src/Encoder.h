#pragma once
#include <Wire.h>

namespace ptz {

class Encoder {
public:
  explicit Encoder(uint8_t addr) : addr_(addr) {}
  int16_t deg() const;

private:
  uint8_t addr_;
};

} // namespace ptz
