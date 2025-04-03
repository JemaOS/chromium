// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/license/license_constants.h"
#include "jemaos/build/config/buildflags.h"

namespace jemaos::constants {

#if BUILDFLAG(USE_JEMAOS_COM)
// URL for the JemaOS license service when using the .com domain
// NOTE: These URLs are placeholders and are not correct for JemaOS.
// They will need to be updated to the appropriate license service URLs.
const char kDefaultJemaOSLicenseUrl[] = "https://cashier.jemaos.com";
#else
// URL for the JemaOS license service when using the .io domain
// NOTE: These URLs are placeholders and are not correct for JemaOS.
// They will need to be updated to the appropriate license service URLs.
const char kDefaultJemaOSLicenseUrl[] = "https://cashier.jemaos.io";
#endif

}  // namespace jemaos::constants