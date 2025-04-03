// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/license/license_switches.h"
#include "base/command_line.h"
#include "jemaos/switches/license/license_constants.h"

namespace jemaos {
namespace switches {

namespace {

// Command-line switch for enabling license test mode
const char kLicenseTestMode[] = "jemaos-license-test-mode";

// Command-line switch for specifying a custom license URL
const char kJemaOSLicenseUrl[] = "jemaos-license-url";

}  // namespace

// Checks if the license test mode is enabled
bool IsLicenseTestMode() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kLicenseTestMode);
}

// Retrieves the JemaOS license URL from the command-line switch or defaults to the constant
std::string GetJemaOSLicenseUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSLicenseUrl)) {
    return command_line->GetSwitchValueASCII(kJemaOSLicenseUrl);
  } else {
    return std::string(jemaos::constants::kDefaultJemaOSLicenseUrl);
  }
}

}  // namespace switches
}  // namespace jemaos