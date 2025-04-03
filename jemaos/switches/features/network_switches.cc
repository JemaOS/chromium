// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/features/network_switches.h"
#include "base/command_line.h"

namespace jemaos {
namespace switches {

namespace {

// Command-line switch to reset the Wi-Fi driver
const char kResetWifiDriver[] = "jemaos-reset-wifi-driver";

}  // namespace

// Checks if the Wi-Fi driver needs to be reset
bool NeedResetWifiDriver() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kResetWifiDriver);
}

}  // namespace switches
}  // namespace jemaos