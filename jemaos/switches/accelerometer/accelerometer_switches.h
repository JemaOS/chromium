// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_ACCELEROMETER_SWITCHES_H_
#define CHROMEOS_JEMAOS_SWITCHES_ACCELEROMETER_SWITCHES_H_

#include "chromeos/chromeos_export.h"

namespace jemaos {
namespace switches {

CHROMEOS_EXPORT bool IsJemaOSAccelerometer();
CHROMEOS_EXPORT int GetAccelConfig();
CHROMEOS_EXPORT bool IsAccelRevertX();
CHROMEOS_EXPORT bool IsAccelRevertY();
CHROMEOS_EXPORT bool IsAccelRevertZ();
CHROMEOS_EXPORT bool IsAccelRightMove();
CHROMEOS_EXPORT int GetAccelRightMoveBits();
CHROMEOS_EXPORT int GetAccelDataPattern();
CHROMEOS_EXPORT bool IsRotate_90();
CHROMEOS_EXPORT bool IsRotate_180();
CHROMEOS_EXPORT bool IsRotate_270();
CHROMEOS_EXPORT int GetJemaOSAccelerometerReadIntervalInMS();
CHROMEOS_EXPORT bool JemaOSAccelerometerSwapBytes();

} // switches
} // jemaos
#endif
