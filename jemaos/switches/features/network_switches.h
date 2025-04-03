// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_NETWORK_SWITCHES_H_
#define CHROMEOS_JEMAOS_SWITCHES_NETWORK_SWITCHES_H_

#include "chromeos/chromeos_export.h"

namespace jemaos {
namespace switches {

// Checks if the Wi-Fi driver needs to be reset
CHROMEOS_EXPORT bool NeedResetWifiDriver();

}  // namespace switches
}  // namespace jemaos

#endif  // CHROMEOS_JEMAOS_SWITCHES_NETWORK_SWITCHES_H_