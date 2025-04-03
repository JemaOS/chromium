// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_LICENSE_SWITCHES_H_
#define CHROMEOS_JEMAOS_SWITCHES_LICENSE_SWITCHES_H_

#include "chromeos/chromeos_export.h"
#include <string>

namespace jemaos {
namespace switches {

// Retrieves the JemaOS license URL
CHROMEOS_EXPORT std::string GetJemaOSLicenseUrl();

// Checks if the license test mode is enabled
CHROMEOS_EXPORT bool IsLicenseTestMode();

}  // namespace switches
}  // namespace jemaos

#endif  // CHROMEOS_JEMAOS_SWITCHES_LICENSE_SWITCHES_H_