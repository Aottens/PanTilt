#pragma once
#include <stdint.h>

// Replace with your 16 byte WiFi primary master key
static const uint8_t WIFI_KEY[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Replace with the MAC addresses of your controller and head boards
static const uint8_t CONTROLLER_MAC[6] = {0, 0, 0, 0, 0, 0};
static const uint8_t HEAD_MAC[6] = {0, 0, 0, 0, 0, 0};
