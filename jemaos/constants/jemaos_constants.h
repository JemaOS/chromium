// Copyright 2024 The Jema Technology Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_CONSTANTS_JEMAOS_CONSTANTS_H_
#define JEMAOS_CONSTANTS_JEMAOS_CONSTANTS_H_

#include "jemaos/build/config/buildflags.h"
#include <string>

namespace jemaos::constants {

#if BUILDFLAG(USE_JEMAOS_LICENSE)

enum class LicenseStateType {
  kUnspecified = 0,
  kUnlicensed = 1,
  kLicenseForYouTrial = 2 ,
  kLicenseForYouValid = 3,
  kLicenseForYouExpired = 4,
  kLicenseForEnterpriseTrial = 5,
  kLicenseForEnterpriseValid = 6,
  kLicenseForEnterpriseExpired = 7,
  kMaxValue = kLicenseForEnterpriseExpired,
};

enum class LicenseEnforcementLevel {
  kNone = 0,
  kForcePopup = 1,
  kForceQuit = 2,
  kMaxValue = kForceQuit,
};

#endif

bool ShouldHideExtensionById(const std::string& extension_id);

} // namespace jemaos::constants

#endif // !JEMAOS_CONSTANTS_JEMAOS_CONSTANTS_H_
