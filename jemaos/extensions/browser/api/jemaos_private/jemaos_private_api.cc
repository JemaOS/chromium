// Copyright 2023 The Jema Technology Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/extensions/browser/api/jemaos_private/jemaos_private_api.h"
#include "jemaos/build/config/buildflags.h"
#include "base/command_line.h"
#include "jemaos/switches/misc/misc_switches.h"

namespace extensions {

ExtensionFunction::ResponseAction JemaosPrivateGetJemaOSInfoFunction::Run() {
  base::Value::Dict result;
  // JEMAOS: this suffix drives the sovereign remote-desktop endpoints used by
  // the preinstalled CRD webapp (accounts.<suffix>, apis.<suffix>/remoting/v1,
  // rtc.<suffix>:5443/bosh). Point it at our own infrastructure.
  std::string host_sufffux = "jematechnology.fr";
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(
        jemaos::switches::kJemaOSServiceHostSuffixForTesting)) {
    host_sufffux = command_line->GetSwitchValueASCII(
        jemaos::switches::kJemaOSServiceHostSuffixForTesting);
  }
  result.Set("host_suffix", host_sufffux);
#if BUILDFLAG(IS_OPENJEMA)
  result.Set("jemaos", false);
#else
  result.Set("jemaos", true);
#endif
  return RespondNow(WithArguments(std::move(result)));
}

}  // namespace extensions
