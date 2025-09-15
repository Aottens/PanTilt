#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <FastAccelStepper.h>
#include <Wire.h>
#include "Config.h"

// MotorController encapsulates stepper motor control for pan, tilt, and zoom axes.
// It provides homing using AS5600 absolute encoders, software limits, and
// synchronized motion to target positions.
class MotorController {
 public:
  MotorController();

  // Initialize steppers, encoders, and I2C bus. Motors remain disabled until homed.
  bool begin();

  // Perform homing by reading absolute encoder positions. Sets current positions
  // and enables motors afterwards.
  void homeAll();

  // Move all axes to given absolute target positions (in steps) so that each
  // axis reaches its destination at the same time using the default speed and
  // acceleration. Limits are enforced.
  void moveTo(int32_t pan, int32_t tilt, int32_t zoom,
              uint32_t speed = DEFAULT_SPEED,
              uint32_t accel = DEFAULT_ACCEL);

  // Jog axes with given speeds (steps/s). Signed values indicate direction.
  void jog(int32_t panSpeed, int32_t tiltSpeed, int32_t zoomSpeed);

  // Enable or disable all motors via the common enable pin.
  void enableMotors(bool enable);

  // Must be called periodically to service steppers (FastAccelStepper uses an
  // ISR but this function can be used for future maintenance tasks).
  void update();

  // Query current positions in steps.
  int32_t panPosition() const;
  int32_t tiltPosition() const;
  int32_t zoomPosition() const;

  // Returns true if any axis is currently moving.
  bool isBusy() const;

 private:
  struct Axis {
    FastAccelStepper* stepper = nullptr;
    uint8_t stepPin = 0;
    uint8_t dirPin = 0;
    uint8_t muxChannel = 0;   // I2C multiplexer channel for encoder
    float stepsPerDegree = 0;  // conversion factor
    int32_t minLimit = 0;
    int32_t maxLimit = 0;
  };

  FastAccelStepperEngine _engine;
  Axis _pan, _tilt, _zoom;

  // Helper to read raw encoder value (0-4095) for a given axis.
  uint16_t readEncoder(uint8_t muxChannel);

  // Convert raw encoder angle to step count.
  int32_t encoderToSteps(uint16_t raw, const Axis& axis);

  // Select channel on I2C multiplexer.
  void selectMuxChannel(uint8_t channel);
};

#endif // MOTOR_CONTROLLER_H
