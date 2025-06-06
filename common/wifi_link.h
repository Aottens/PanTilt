#pragma once
#include <Arduino.h>
#include <esp_now.h>
#include <functional>

namespace ptz {

class WiFiLink {
public:
  using RecvCallback = std::function<void(const uint8_t *data, size_t len)>;

  // Initialize ESP-NOW in station mode. Does not connect to an AP.
  bool beginSTA(const uint8_t peer[6], RecvCallback cb);
  // Initialize ESP-NOW in soft AP mode for the controller.
  bool beginAP(RecvCallback cb);
  bool send(const uint8_t peer[6], const void *msg, size_t len);

private:
  static void onRecv(const uint8_t *mac, const uint8_t *data, int len);
  static WiFiLink *instance_;
  RecvCallback callback_;
};

} // namespace ptz
