#include "MotionController.h"
#include "ptz_proto.h"
#include "wifi_link.h"
#include <Arduino.h>
#include <Wire.h>

using namespace ptz;

static WiFiLink wifiLink;
static MotionController motion;
static uint8_t controllerMac[6] = {0, 0, 0, 0, 0, 0};

static void onRecv(const uint8_t *data, size_t len) {
  if (len == sizeof(HeadCmd)) {
    auto cmd = *reinterpret_cast<const HeadCmd *>(data);
    motion.setTarget(cmd.pan_deg, cmd.tilt_deg, cmd.vel_pct);
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  motion.begin();
  wifiLink.beginSTA(controllerMac, onRecv);
}

unsigned long lastStatus = 0;
void loop() {
  motion.task();
  if (millis() - lastStatus >= 10) {
    HeadStatus st{motion.panDeg(), motion.tiltDeg(), 0, motion.flags()};
    wifiLink.send(controllerMac, &st, sizeof(st));
    lastStatus = millis();
  }
  delay(1);
}
