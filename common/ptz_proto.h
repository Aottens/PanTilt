#pragma once
#include <cstddef>
#include <stdint.h>

namespace ptz {

enum class MsgType : uint8_t { HEAD_CMD = 1, HEAD_STATUS = 2 };

#pragma pack(push, 1)
struct HeadCmd {
  int16_t pan_deg;
  int16_t tilt_deg;
  int16_t vel_pct;
};

struct HeadStatus {
  int16_t pan_deg;
  int16_t tilt_deg;
  uint8_t vel_pct;
  uint8_t flags;
};
#pragma pack(pop)

constexpr int16_t PAN_MIN = -135;
constexpr int16_t PAN_MAX = 135;
constexpr int16_t TILT_MIN = -120;
constexpr int16_t TILT_MAX = 120;

constexpr uint8_t FLAG_ENCODER_ERR = 0x01;

constexpr uint16_t crc16_byte(uint16_t crc, uint8_t data) {
  crc ^= data;
  for (int j = 0; j < 8; ++j) {
    crc = (crc & 1) ? (crc >> 1) ^ 0xA001 : crc >> 1;
  }
  return crc;
}

template <size_t N>
constexpr uint16_t crc16_calc(const uint8_t (&data)[N], size_t idx = 0,
                              uint16_t crc = 0xFFFF) {
  return idx == N ? crc
                  : crc16_calc<N>(data, idx + 1, crc16_byte(crc, data[idx]));
}

template <typename T> constexpr uint16_t crc16_struct() {
  union U {
    T obj;
    uint8_t bytes[sizeof(T)];
    constexpr U() : bytes{} {}  // keep 'bytes' active for constant evaluation
  } u;
  } u{};
  return crc16_calc(u.bytes);
}

constexpr HeadCmd empty_cmd{};
constexpr HeadStatus empty_status{};
constexpr uint16_t HEAD_CMD_CRC = crc16_struct<HeadCmd>();
constexpr uint16_t HEAD_STATUS_CRC = crc16_struct<HeadStatus>();

static_assert(sizeof(HeadCmd) == 6, "HeadCmd size mismatch");
static_assert(sizeof(HeadStatus) == 6, "HeadStatus size mismatch");

} // namespace ptz
