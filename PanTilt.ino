#include <Arduino.h>
#include "Config.h"
#include "MotorController.h"
#include "KeyframeManager.h"
#include "ControllerInput.h"
#include "PlaybackEngine.h"

MotorController motorController;
KeyframeManager keyframeManager;
PlaybackEngine playbackEngine;
ControllerInput controllerInput;

void setup() {
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial) ;
  Serial.println("PTZ firmware starting...");
#endif

  pinMode(DRIVER_ENABLE_PIN, OUTPUT);
  motorController.begin();
  keyframeManager.begin();
  playbackEngine.begin(&motorController, &keyframeManager);
  controllerInput.begin(&motorController, &keyframeManager, &playbackEngine);

  motorController.homeAll();
}

void loop() {
  controllerInput.update();
  playbackEngine.update();
  motorController.update();
  delay(10); // basic scheduler tick
}

// Placeholder hooks for future extensions:
// - Focus motor control
// - WiFi/MQTT/AsyncWebServer interfaces
