// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_SERVICES_SWITCHES_H_
#define CHROMEOS_JEMAOS_SWITCHES_SERVICES_SWITCHES_H_

#include <string>
#include "chromeos/chromeos_export.h"
#include "jemaos/build/config/buildflags.h"

namespace jemaos {
namespace switches {

CHROMEOS_EXPORT bool DisableJemaOSGeolocationAPI();
CHROMEOS_EXPORT bool DisableJemaOSTimezoneAPI();

extern std::string GetJemaOSGeolocationAPIUrl();
extern std::string GetJemaOSTimezoneAPIUrl();

extern std::string GetJemaOSLookingGlassUrl();

extern std::string GetJemaOSAppStoreURL();

extern std::string GetJemaOSWebStoreUpdateUrl();

extern std::string GetJemaOSAssistantWebUrl();

CHROMEOS_EXPORT std::string MayConvertWebStoreUpdateUrl(
    const std::string& url);

#if BUILDFLAG(JEMAOS_DEVICE)
std::string GetJemaOSProductWarrantyUrl();
#endif

} // switches
} // jemaos

#endif
