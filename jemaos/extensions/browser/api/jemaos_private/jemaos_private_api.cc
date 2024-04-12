// Copyright 2023 The Jema Innovations Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/extensions/browser/api/jemaos_private/jemaos_private_api.h"
#include "jemaos/build/config/buildflags.h"
#include "base/command_line.h"
#include "jemaos/switches/misc/misc_switches.h"

namespace extensions {

ExtensionFunction::ResponseAction JemaosPrivateGetJemaOSInfoFunction::Run() {
  base::Value result(base::Value::Type::DICT);
#if BUILDFLAG(USE_JEMAOS_COM)
  std::string host_sufffux = "jemaos.com";
#else
  std::string host_sufffux = "jemaos.io";
#endif
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(
        jemaos::switches::kJemaOSServiceHostSuffixForTesting)) {
    host_sufffux = command_line->GetSwitchValueASCII(
        jemaos::switches::kJemaOSServiceHostSuffixForTesting);
  }
  result.SetStringKey("host_suffix", host_sufffux);
#if BUILDFLAG(IS_OPENJEMA)
  result.SetBoolKey("jemaos", false);
#else
  result.SetBoolKey("jemaos", true);
#endif
  return RespondNow(WithArguments(std::move(result)));
}

}  // namespace extensions
