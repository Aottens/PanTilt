# PanTilt

Firmware for an ESP32 based pan/tilt head and its remote controller.

## Implemented MUST features
- ESP-NOW communication with AES encryption (no Wi-Fi association required)
- Common PTZ protocol with compile time CRC
- Motion controller with encoder watchdog and soft limits
- Joystick based controller UI on OLED display

## Building
Install PlatformIO and run:
```bash
platformio run -e head        # build head firmware
platformio run -e controller  # build controller firmware
```

Upload with `--target upload` as usual.

## License
This project is licensed under the [MIT License](LICENSE).
