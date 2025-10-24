// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_LICENSE_SWITCHES_H_
#define CHROMEOS_JEMAOS_SWITCHES_LICENSE_SWITCHES_H_

#include "chromeos/chromeos_export.h"
#include <string>

namespace jemaos {
namespace switches {

extern std::string GetJemaOSLicenseWebUrl();
extern std::string GetJemaOSLicenseApiUrl();
CHROMEOS_EXPORT bool IsLicenseTestMode();

} // switches
} // jemaos

#endif
