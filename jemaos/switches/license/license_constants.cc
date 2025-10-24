// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/license/license_constants.h"
#include "jemaos/build/config/buildflags.h"

namespace jemaos::constants {

#if BUILDFLAG(USE_JEMAOS_COM)
const char kDefaultJemaOSLicenseWebUrl[] = "https://cashier.jemaos.com";
const char kDefaultJemaOSLicenseApiUrl[] = "https://apis.jemaos.com/license";
#else
const char kDefaultJemaOSLicenseWebUrl[] = "https://cashier.jemaos.io";
const char kDefaultJemaOSLicenseApiUrl[] = "https://apis.jemaos.io/license";
#endif

const char kJemaOSOEMTokenFilePath[] = "/usr/share/oem/jemaos_oem_token";

}  // namespace jemaos::constants
