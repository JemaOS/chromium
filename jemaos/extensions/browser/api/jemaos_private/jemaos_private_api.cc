// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/extensions/browser/api/jemaos_private/jemaos_private_api.h"
#include "jemaos/build/config/buildflags.h"
#include "base/command_line.h"
#include "jemaos/switches/misc/misc_switches.h"

namespace extensions {

// Handles the "GetJemaOSInfo" function
ExtensionFunction::ResponseAction JemaosPrivateGetJemaOSInfoFunction::Run() {
  base::Value result(base::Value::Type::DICT);

  // Determine the host suffix based on build flags
#if BUILDFLAG(USE_JEMAOS_COM)
  std::string host_sufffux = "jemaos.com";
#else
  std::string host_sufffux = "jemaos.io";
#endif

  // Check if a custom host suffix is provided via command-line switch
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(
        jemaos::switches::kJemaOSServiceHostSuffixForTesting)) {
    host_sufffux = command_line->GetSwitchValueASCII(
        jemaos::switches::kJemaOSServiceHostSuffixForTesting);
  }

  // Set the host suffix in the result
  result.SetStringKey("host_suffix", host_sufffux);

  // Determine if the build is for OpenJema or JemaOS
#if BUILDFLAG(IS_OPENJEMA)
  result.SetBoolKey("jemaos", false);
#else
  result.SetBoolKey("jemaos", true);
#endif

  // Respond with the result
  return RespondNow(WithArguments(std::move(result)));
}

}  // namespace extensions