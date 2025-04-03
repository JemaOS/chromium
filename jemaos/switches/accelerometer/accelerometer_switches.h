// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_ACCELEROMETER_SWITCHES_H_
#define CHROMEOS_JEMAOS_SWITCHES_ACCELEROMETER_SWITCHES_H_

#include "chromeos/chromeos_export.h"

namespace jemaos {
namespace switches {

// Checks if the JemaOS accelerometer is enabled
CHROMEOS_EXPORT bool IsJemaOSAccelerometer();

// Retrieves the accelerometer configuration index
CHROMEOS_EXPORT int GetAccelConfig();

// Checks if the X-axis is reverted
CHROMEOS_EXPORT bool IsAccelRevertX();

// Checks if the Y-axis is reverted
CHROMEOS_EXPORT bool IsAccelRevertY();

// Checks if the Z-axis is reverted
CHROMEOS_EXPORT bool IsAccelRevertZ();

// Checks if the accelerometer right move is enabled
CHROMEOS_EXPORT bool IsAccelRightMove();

// Retrieves the number of right move bits for the accelerometer
CHROMEOS_EXPORT int GetAccelRightMoveBits();

// Retrieves the accelerometer data pattern
CHROMEOS_EXPORT int GetAccelDataPattern();

// Checks if the accelerometer is rotated by 90 degrees
CHROMEOS_EXPORT bool IsRotate_90();

// Checks if the accelerometer is rotated by 180 degrees
CHROMEOS_EXPORT bool IsRotate_180();

// Checks if the accelerometer is rotated by 270 degrees
CHROMEOS_EXPORT bool IsRotate_270();

// Retrieves the accelerometer read interval in milliseconds
CHROMEOS_EXPORT int GetJemaOSAccelerometerReadIntervalInMS();

// Retrieves the accelerometer initialization timeout in seconds
CHROMEOS_EXPORT int GetJemaOSAccelerometerInitializeTimeoutInSeconds();

// Checks if the accelerometer bytes should be swapped
CHROMEOS_EXPORT bool JemaOSAccelerometerSwapBytes();

}  // namespace switches
}  // namespace jemaos

#endif  // CHROMEOS_JEMAOS_SWITCHES_ACCELEROMETER_SWITCHES_H_