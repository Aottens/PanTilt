# PanTilt

Firmware for an ESP32 based pan/tilt head and its remote controller.

## Implemented MUST features
- ESP-NOW communication with AES encryption
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

## Soft limit menu
When the controller starts it enters a menu to adjust the allowed ranges.
Move the joystick up or down to select `panMin`, `panMax`, `tiltMin` or
`tiltMax`. Push the stick left or right to decrease or increase the value.
After a few seconds with no input the menu closes and the limits are saved.

## License
This project is licensed under the [MIT License](LICENSE).
