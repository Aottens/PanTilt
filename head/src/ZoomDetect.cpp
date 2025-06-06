#include "ZoomDetect.h"

namespace ptz {

bool ZoomDetect::zoomPresent() {
  Wire.beginTransmission(0x40);
  return Wire.endTransmission() == 0;
}

} // namespace ptz
