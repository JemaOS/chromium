// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/license/license_constants.h"
#include "jemaos/build/config/buildflags.h"

namespace jemaos::constants {

#if BUILDFLAG(USE_JEMAOS_COM)
const char kDefaultJemaOSLicenseUrl[] = "https://cashier.jemaos.com";
#else
const char kDefaultJemaOSLicenseUrl[] = "https://cashier.jemaos.io";
#endif

}  // namespace jemaos::constants
