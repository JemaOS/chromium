// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_DISPLAY_SWITCHES_H_
#define CHROMEOS_JEMAOS_SWITCHES_DISPLAY_SWITCHES_H_

#include "chromeos/chromeos_export.h"

namespace jemaos {
namespace switches {

// Retrieves the default device scale factor from the command-line switch
CHROMEOS_EXPORT float GetDefaultDSF(float default_value);

// Retrieves the default screen DPI from the command-line switch
CHROMEOS_EXPORT float GetDefaultScreenDpi(float default_value);

}  // namespace switches
}  // namespace jemaos

#endif  // CHROMEOS_JEMAOS_SWITCHES_DISPLAY_SWITCHES_H_