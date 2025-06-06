#include "OledUI.h"

namespace ptz {

void OledUI::begin() {
  display_.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display_.clearDisplay();
  display_.setTextColor(SSD1306_WHITE);
}

void OledUI::update(const HeadStatus &st, bool zoom) {
  display_.clearDisplay();
  display_.setCursor(0, 0);
  char buf[32];
  snprintf(buf, sizeof(buf), "PAN %+04d\xB0", st.pan_deg);
  display_.println(buf);
  snprintf(buf, sizeof(buf), "TILT %+04d\xB0", st.tilt_deg);
  display_.println(buf);
  snprintf(buf, sizeof(buf), "SPD %3d %%", st.vel_pct);
  display_.println(buf);
  if (!zoom)
    display_.println("ZOOM --");
  display_.display();
}

} // namespace ptz
