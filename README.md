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


## Testing
Run the unit tests using PlatformIO's native environment:

```bash
platformio test -e native
```

This will build and execute the tests on your host machine using Google Test.

## Soft limit menu
When the controller starts it enters a menu to adjust the allowed ranges.
Move the joystick up or down to select `panMin`, `panMax`, `tiltMin` or
`tiltMax`. Push the stick left or right to decrease or increase the value.
After a few seconds with no input the menu closes and the limits are saved.

### Private WiFi key
Copy `common/secrets.example.h` to `common/secrets.h` and edit the
`WIFI_KEY` array with your own 16‑byte key before building. The file also
contains placeholders for `CONTROLLER_MAC` and `HEAD_MAC` which you may fill
with the MAC addresses of your boards. The `secrets.h` file is excluded from
version control via `.gitignore`.

## Configuring MAC addresses
Each device needs to know the MAC address of its peer. You can read the
hardware MAC of an ESP32 by uploading the following sketch and checking the
serial output:

```cpp
#include <WiFi.h>
void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());
}
void loop() {}
```

Copy the printed address of each board into `common/secrets.h` (variables
`CONTROLLER_MAC` and `HEAD_MAC`) or store them in Preferences using the keys
`controller` and `head` under the `wifi` namespace before flashing the final
firmware.

## License
This project is licensed under the [MIT License](LICENSE).
