// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/arc/arc_switches.h"
#include "base/command_line.h"
#include <string>

namespace jemaos {
namespace switches {

namespace {

const char kJemaOSArcDelay[] ="jemaos-arc-delay";

}

int64_t GetJemaOSArcDelay() {
	std::string delayStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kJemaOSArcDelay);
	if (delayStr.empty())
		return 0;
	return (int64_t) std::stoi(delayStr);
}

} // switches
} // jemaos

