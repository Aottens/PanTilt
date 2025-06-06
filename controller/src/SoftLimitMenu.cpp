#include "SoftLimitMenu.h"
#include "ptz_proto.h"
#include <Arduino.h>

namespace ptz {

void SoftLimitMenu::begin() {
  prefs_.begin("limits", false);
  limits_.panMin = prefs_.getShort("panMin", PAN_MIN);
  limits_.panMax = prefs_.getShort("panMax", PAN_MAX);
  limits_.tiltMin = prefs_.getShort("tiltMin", TILT_MIN);
  limits_.tiltMax = prefs_.getShort("tiltMax", TILT_MAX);
}

void SoftLimitMenu::edit(Joystick &joy, Adafruit_SSD1306 &disp) {
  disp.clearDisplay();
  disp.setTextColor(SSD1306_WHITE);
  int selection = 0;
  unsigned long lastInput = millis();
  auto redraw = [&]() {
    disp.clearDisplay();
    disp.setCursor(0, 0);
    char buf[32];
    snprintf(buf, sizeof(buf), "%c panMin %+04d", selection == 0 ? '>' : ' ',
             limits_.panMin);
    disp.println(buf);
    snprintf(buf, sizeof(buf), "%c panMax %+04d", selection == 1 ? '>' : ' ',
             limits_.panMax);
    disp.println(buf);
    snprintf(buf, sizeof(buf), "%c tiltMin %+04d", selection == 2 ? '>' : ' ',
             limits_.tiltMin);
    disp.println(buf);
    snprintf(buf, sizeof(buf), "%c tiltMax %+04d", selection == 3 ? '>' : ' ',
             limits_.tiltMax);
    disp.println(buf);
    disp.display();
  };
  redraw();

  while (millis() - lastInput < 5000) {
    int16_t dx = joy.x();
    int16_t dy = joy.y();
    if (abs(dy) > 600) {
      selection += dy > 0 ? 1 : -1;
      selection = constrain(selection, 0, 3);
      redraw();
      lastInput = millis();
    }
    if (abs(dx) > 600) {
      int delta = dx > 0 ? 1 : -1;
      switch (selection) {
      case 0:
        limits_.panMin = constrain(limits_.panMin + delta, PAN_MIN, PAN_MAX);
        break;
      case 1:
        limits_.panMax = constrain(limits_.panMax + delta, PAN_MIN, PAN_MAX);
        break;
      case 2:
        limits_.tiltMin =
            constrain(limits_.tiltMin + delta, TILT_MIN, TILT_MAX);
        break;
      case 3:
        limits_.tiltMax =
            constrain(limits_.tiltMax + delta, TILT_MIN, TILT_MAX);
        break;
      }
      redraw();
      lastInput = millis();
    }
    delay(150);
  }

  prefs_.putShort("panMin", limits_.panMin);
  prefs_.putShort("panMax", limits_.panMax);
  prefs_.putShort("tiltMin", limits_.tiltMin);
  prefs_.putShort("tiltMax", limits_.tiltMax);
}

} // namespace ptz
