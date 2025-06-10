#include "MotionController.h"
#include "ptz_proto.h"
#include "ZoomDetect.h"
#include "wifi_link.h"
#include "secrets.h"
#include <Arduino.h>
#include <Preferences.h>
#include <Wire.h>

using namespace ptz;

static WiFiLink wifiLink;
static MotionController motion;
static ZoomDetect zoomDet;
static bool zoomPresent = false;
static Preferences prefs;
static uint8_t controllerMac[6];

static void onRecv(const uint8_t *data, size_t len) {
  if (len == sizeof(HeadCmd)) {
    auto cmd = *reinterpret_cast<const HeadCmd *>(data);
    motion.setTarget(cmd.pan_deg, cmd.tilt_deg, cmd.vel_pct);
  }
}

void setup() {
  Serial.begin(115200);
  prefs.begin("wifi", true);
  if (prefs.getBytes("controller", controllerMac, 6) != 6) {
    memcpy(controllerMac, CONTROLLER_MAC, 6);
  }
  prefs.end();
  static const uint8_t unset[6] = {0, 0, 0, 0, 0, 0};
  if (memcmp(controllerMac, unset, 6) == 0) {
    Serial.println("WARNING: controller MAC address not configured");
  }
  Wire.begin();
  zoomPresent = zoomDet.zoomPresent();
  motion.begin();
  wifiLink.beginSTA(controllerMac, onRecv);
}

unsigned long lastStatus = 0;
void loop() {
  motion.task();
  if (millis() - lastStatus >= 10) {
    uint8_t flags = motion.flags();
    if (zoomPresent)
      flags |= FLAG_ZOOM_PRESENT;
    HeadStatus st{motion.panDeg(), motion.tiltDeg(), 0, flags};
    wifiLink.send(controllerMac, &st, sizeof(st));
    lastStatus = millis();
  }
  delay(1);
}
