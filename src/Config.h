#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Debugging macro. Uncomment to enable debug prints.
#define DEBUG 1

// Pin definitions for stepper drivers
constexpr uint8_t PAN_STEP_PIN = 1;   // IO-1
constexpr uint8_t PAN_DIR_PIN  = 2;   // IO-2
constexpr uint8_t TILT_STEP_PIN = 4;  // IO-4
constexpr uint8_t TILT_DIR_PIN  = 5;  // IO-5
constexpr uint8_t ZOOM_STEP_PIN = 6;  // IO-6
constexpr uint8_t ZOOM_DIR_PIN  = 7;  // IO-7
constexpr uint8_t DRIVER_ENABLE_PIN = 12; // IO-12 common enable

// I2C pins for encoder multiplexer
constexpr uint8_t I2C_SDA_PIN = 8;  // IO-8
constexpr uint8_t I2C_SCL_PIN = 9;  // IO-9

// I2C multiplexer (e.g., TCA9548A) address
constexpr uint8_t I2C_MUX_ADDR = 0x70;

// Encoder I2C address (AS5600)
constexpr uint8_t AS5600_ADDR = 0x36;

// Mapping of multiplexer channels to axes
constexpr uint8_t PAN_ENCODER_CH  = 0;
constexpr uint8_t TILT_ENCODER_CH = 1;
constexpr uint8_t ZOOM_ENCODER_CH = 2;

// Motion parameters
constexpr uint32_t DEFAULT_SPEED = 2000;      // steps per second
constexpr uint32_t DEFAULT_ACCEL = 800;       // steps per second^2
constexpr uint32_t MAX_JOG_SPEED = 1500;      // max speed from joystick

// Steps per degree for each axis (adjust according to mechanics)
constexpr float PAN_STEPS_PER_DEG  = 10.0f;
constexpr float TILT_STEPS_PER_DEG = 10.0f;
constexpr float ZOOM_STEPS_PER_MM  = 50.0f;   // example scale for zoom

// Software limits in steps
constexpr int32_t PAN_MIN  = -3600; // -360 degrees
constexpr int32_t PAN_MAX  =  3600; // 360 degrees
constexpr int32_t TILT_MIN = -900;  // -90 degrees
constexpr int32_t TILT_MAX =  900;  // 90 degrees
constexpr int32_t ZOOM_MIN = 0;     // min zoom
constexpr int32_t ZOOM_MAX = 2000;  // max zoom

// Number of keyframes supported
constexpr uint8_t MAX_KEYFRAMES = 4;

#endif // CONFIG_H
