#include "Encoder.h"

namespace ptz {

int16_t Encoder::deg() const {
  Wire.beginTransmission(addr_);
  Wire.write(0x0C);
  if (Wire.endTransmission(false) != 0)
    return 0;
  Wire.requestFrom(addr_, (uint8_t)2);
  if (Wire.available() < 2)
    return 0;
  uint16_t raw = (Wire.read() << 8) | Wire.read();
  float deg = raw * 0.08789f;                    // 360/4096
  return static_cast<int16_t>(deg + 0.5f) - 180; // centered around 0
}

} // namespace ptz
