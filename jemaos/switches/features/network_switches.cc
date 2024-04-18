// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/features/network_switches.h"
#include "base/command_line.h"

namespace jemaos {
namespace switches {

namespace {

const char kResetWifiDriver[] = "jemaos-reset-wifi-driver";

}

bool NeedResetWifiDriver() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kResetWifiDriver);
}

} // switches
} // jemaos

