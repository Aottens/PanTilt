# PanTilt

Firmware for an ESP32 WROOM based pan/tilt camera head. The code is intended
for use with [PlatformIO](https://platformio.org/) and the Arduino framework.

## Features
- Wi-Fi configuration via temporary access point and simple web page.
- Connection status shown on an OLED display (SSD1306).
- Basic HTTP endpoint to control pan, tilt and zoom motors.
- Position feedback using three AS5600 magnetic encoders.


## Building
Install PlatformIO and run the build from this directory:

```bash
platformio run
```

Upload the firmware using:

```bash
platformio run --target upload
```

The configuration for PlatformIO is stored in `platformio.ini`.
