#include "wifi_link.h"
#include "secrets.h"
#include <WiFi.h>

namespace ptz {

WiFiLink *WiFiLink::instance_ = nullptr;

static void onSent(const uint8_t *, esp_now_send_status_t) {}

bool WiFiLink::beginSTA(const uint8_t peer[6], RecvCallback cb) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false, true);
  if (!WiFi.begin()) {
    return false;
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
  }
  if (esp_now_init() != ESP_OK)
    return false;
  esp_now_register_send_cb(onSent);
  esp_now_register_recv_cb(&WiFiLink::onRecv);
  esp_now_set_pmk(WIFI_KEY);
  esp_now_peer_info_t info{};
  memcpy(info.peer_addr, peer, 6);
  info.channel = 0;
  info.encrypt = true;
  memcpy(info.lmk, WIFI_KEY, 16);
  esp_now_add_peer(&info);
  callback_ = cb;
  instance_ = this;
  return true;
}

bool WiFiLink::beginAP(RecvCallback cb) {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ptz-controller");
  if (esp_now_init() != ESP_OK)
    return false;
  esp_now_register_send_cb(onSent);
  esp_now_register_recv_cb(&WiFiLink::onRecv);
  esp_now_set_pmk(WIFI_KEY);
  callback_ = cb;
  instance_ = this;
  return true;
}

bool WiFiLink::send(const uint8_t peer[6], const void *msg, size_t len) {
  return esp_now_send(peer, static_cast<const uint8_t *>(msg), len) == ESP_OK;
}

void WiFiLink::onRecv(const uint8_t *mac, const uint8_t *data, int len) {
  if (instance_ && instance_->callback_) {
    instance_->callback_(data, len);
  }
}

} // namespace ptz
