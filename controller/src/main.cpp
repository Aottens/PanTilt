#include "Joystick.h"
#include "OledUI.h"
#include "SoftLimitMenu.h"
#include "ptz_proto.h"
#include "wifi_link.h"
#include <Arduino.h>

using namespace ptz;

static WiFiLink wifiLink;
static Joystick joy;
static OledUI ui;
static SoftLimitMenu menu;
static uint8_t headMac[6] = {0, 0, 0, 0, 0, 0};
static HeadStatus lastStatus{};

static void onRecv(const uint8_t *data, size_t len) {
  if (len == sizeof(HeadStatus)) {
    lastStatus = *reinterpret_cast<const HeadStatus *>(data);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  joy.begin(34, 35);
  ui.begin();
  menu.begin();
  menu.edit(joy, ui.display());
  wifiLink.beginAP(onRecv);
}

unsigned long lastSend = 0;
void loop() {
  int16_t v = joy.velocity();
  if (millis() - lastSend >= 20) {
    HeadCmd cmd{0, 0, v};
    wifiLink.send(headMac, &cmd, sizeof(cmd));
    lastSend = millis();
  }
  ui.update(lastStatus, true);
  delay(10);
}
