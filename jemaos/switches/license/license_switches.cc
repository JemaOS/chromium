// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/license/license_switches.h"
#include "base/command_line.h"
#include "jemaos/switches/license/license_constants.h"

namespace jemaos {
namespace switches {

namespace {

const char kLicenseTestMode[] = "jemaos-license-test-mode";
const char kJemaOSLicenseUrl[] = "jemaos-license-url";

}

bool IsLicenseTestMode() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kLicenseTestMode);
}

std::string GetJemaOSLicenseUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSLicenseUrl)) {
    return command_line->GetSwitchValueASCII(kJemaOSLicenseUrl);
  } else {
    return std::string(jemaos::constants::kDefaultJemaOSLicenseUrl);
  }
}

} // switches
} // jemaos
