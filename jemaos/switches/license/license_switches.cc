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
const char kJemaOSLicenseWebUrl[] = "jemaos-license-web-url";
const char kJemaOSLicenseApiUrl[] = "jemaos-license-api-url";

}

bool IsLicenseTestMode() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kLicenseTestMode);
}

std::string GetJemaOSLicenseWebUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSLicenseWebUrl)) {
    return command_line->GetSwitchValueASCII(kJemaOSLicenseWebUrl);
  } else {
    return std::string(jemaos::constants::kDefaultJemaOSLicenseWebUrl);
  }
}

std::string GetJemaOSLicenseApiUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSLicenseApiUrl)) {
    return command_line->GetSwitchValueASCII(kJemaOSLicenseApiUrl);
  } else {
    return std::string(jemaos::constants::kDefaultJemaOSLicenseApiUrl);
  }
}

} // switches
} // jemaos
