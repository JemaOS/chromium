// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/display/display_switches.h"

#include "base/command_line.h"
#include <string>

namespace jemaos {
namespace switches {

namespace {

const char kDefaultDSF[] = "jemaos-default-dsf";
const char kScreenDpi[] = "jemaos-default-screen-dpi";

}

float GetDefaultDSF(float default_value) {
  std::string factorStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kDefaultDSF);
  if (factorStr.empty())
    return default_value;
  return std::stof(factorStr);
}

float GetDefaultScreenDpi(float default_value) {
  std::string factorStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kScreenDpi);
  if (factorStr.empty())
    return default_value;
  return std::stof(factorStr);
}

} // switches
} // jemaos

