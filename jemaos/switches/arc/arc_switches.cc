// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/arc/arc_switches.h"
#include "base/command_line.h"
#include <string>

namespace jemaos {
namespace switches {

namespace {

// Command-line switch for ARC delay
const char kJemaOSArcDelay[] = "jemaos-arc-delay";

}  // namespace

// Retrieves the ARC delay value from the command-line switch
int64_t GetJemaOSArcDelay() {
  std::string delayStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kJemaOSArcDelay);
  if (delayStr.empty())
    return 0;
  return static_cast<int64_t>(std::stoi(delayStr));
}

}  // namespace switches
}  // namespace jemaos