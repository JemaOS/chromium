// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/accelerometer/accelerometer_switches.h"
#include "base/command_line.h"

namespace jemaos {
namespace switches {

namespace {

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
const char kJemaOSAccelerometerSwapBytes[] = "jemaos-accel-swap-bytes";

}

bool IsJemaOSAccelerometer() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kAccelConfigIndex);
}

int GetAccelConfig() {
  std::string indexStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kAccelConfigIndex);
  if (indexStr.empty())
    return 0;
  return std::stoi(indexStr);
}

bool IsAccelRevertX() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kAccelRevertX);
}

bool IsAccelRevertY() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kAccelRevertY);
}

bool IsAccelRevertZ() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kAccelRevertZ);
}

bool IsAccelRightMove() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kAccelRightMoveBits);
}

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

int GetAccelDataPattern() {
  std::string patternStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kAccelDataPattern);
  if (patternStr.empty())
    return 0;
  return std::stoi(patternStr);
}

bool IsRotate_90(){
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kRotate_90);
}

bool IsRotate_180(){
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kRotate_180);
}

bool IsRotate_270(){
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kRotate_270);
}

int GetJemaOSAccelerometerReadIntervalInMS() {
  std::string patternStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kJemaOSAccelerometerReadInterval);
  if (patternStr.empty())
    return 0;
  return std::stoi(patternStr);
}

bool JemaOSAccelerometerSwapBytes() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kJemaOSAccelerometerSwapBytes);
}

} // switches
} // jemaos
