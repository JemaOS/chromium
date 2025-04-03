// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/display/display_switches.h"

#include "base/command_line.h"
#include <string>

namespace jemaos {
namespace switches {

namespace {

// Command-line switch for the default device scale factor
const char kDefaultDSF[] = "jemaos-default-dsf";

// Command-line switch for the default screen DPI
const char kScreenDpi[] = "jemaos-default-screen-dpi";

}  // namespace

// Retrieves the default device scale factor from the command-line switch
float GetDefaultDSF(float default_value) {
  std::string factorStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kDefaultDSF);
  if (factorStr.empty())
    return default_value;
  return std::stof(factorStr);
}

// Retrieves the default screen DPI from the command-line switch
float GetDefaultScreenDpi(float default_value) {
  std::string factorStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kScreenDpi);
  if (factorStr.empty())
    return default_value;
  return std::stof(factorStr);
}

}  // namespace switches
}  // namespace jemaos