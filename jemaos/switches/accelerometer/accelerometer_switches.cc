// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/accelerometer/accelerometer_switches.h"
#include "base/command_line.h"

namespace jemaos {
namespace switches {

namespace {

// Command-line switches for accelerometer configuration
const char kAccelConfigIndex[] = "jemaos-accel-config";
const char kAccelRevertX[] = "jemaos-accel-revert-x";
const char kAccelRevertY[] = "jemaos-accel-revert-y";
const char kAccelRevertZ[] = "jemaos-accel-revert-z";
const char kAccelRightMoveBits[] = "jemaos-accel-right-move";
const char kAccelDataPattern[] = "jemaos-accel-pattern";
const char kRotate_90[] = "jemaos-rotate-90";
const char kRotate_180[] = "jemaos-rotate-180";
const char kRotate_270[] = "jemaos-rotate-270";
const char kJemaOSAccelerometerReadInterval[] = "jemaos-accel-read-interval";
const char kJemaOSAccelerometerInitializeTimeoutInSeconds[] = "jemaos-accel-initialize-timeout";
const char kJemaOSAccelerometerSwapBytes[] = "jemaos-accel-swap-bytes";

}  // namespace

// Checks if the JemaOS accelerometer is enabled
bool IsJemaOSAccelerometer() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kAccelConfigIndex);
}

// Retrieves the accelerometer configuration index
int GetAccelConfig() {
  std::string indexStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kAccelConfigIndex);
  if (indexStr.empty())
    return 0;
  return std::stoi(indexStr);
}

// Checks if the X-axis is reverted
bool IsAccelRevertX() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kAccelRevertX);
}

// Checks if the Y-axis is reverted
bool IsAccelRevertY() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kAccelRevertY);
}

// Checks if the Z-axis is reverted
bool IsAccelRevertZ() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kAccelRevertZ);
}

// Checks if the accelerometer right move is enabled
bool IsAccelRightMove() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kAccelRightMoveBits);
}

// Retrieves the number of right move bits for the accelerometer
int GetAccelRightMoveBits() {
  int ret = 0;
  if (IsAccelRightMove()) {
    ret = 4;
    std::string rightBitsStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kAccelRightMoveBits);
    if (!rightBitsStr.empty())
      ret = std::stoi(rightBitsStr);
  }
  return ret;
}

// Retrieves the accelerometer data pattern
int GetAccelDataPattern() {
  std::string patternStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kAccelDataPattern);
  if (patternStr.empty())
    return 0;
  return std::stoi(patternStr);
}

// Checks if the accelerometer is rotated by 90 degrees
bool IsRotate_90() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kRotate_90);
}

// Checks if the accelerometer is rotated by 180 degrees
bool IsRotate_180() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kRotate_180);
}

// Checks if the accelerometer is rotated by 270 degrees
bool IsRotate_270() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kRotate_270);
}

// Retrieves the accelerometer read interval in milliseconds
int GetJemaOSAccelerometerReadIntervalInMS() {
  std::string patternStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kJemaOSAccelerometerReadInterval);
  if (patternStr.empty())
    return 0;
  return std::stoi(patternStr);
}

// Checks if the accelerometer bytes should be swapped
bool JemaOSAccelerometerSwapBytes() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kJemaOSAccelerometerSwapBytes);
}

// Retrieves the accelerometer initialization timeout in seconds
int GetJemaOSAccelerometerInitializeTimeoutInSeconds() {
  std::string str = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kJemaOSAccelerometerInitializeTimeoutInSeconds);
  if (str.empty())
    return 0;
  return std::stoi(str);
}

}  // namespace switches
}  // namespace jemaos