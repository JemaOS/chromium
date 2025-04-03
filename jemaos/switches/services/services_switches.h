// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_SERVICES_SWITCHES_H_
#define CHROMEOS_JEMAOS_SWITCHES_SERVICES_SWITCHES_H_

#include <string>
#include "chromeos/chromeos_export.h"

namespace jemaos {
namespace switches {

// Checks if the Geolocation API is disabled
CHROMEOS_EXPORT bool DisableJemaOSGeolocationAPI();

// Checks if the Timezone API is disabled
CHROMEOS_EXPORT bool DisableJemaOSTimezoneAPI();

// Retrieves the Geolocation API URL
CHROMEOS_EXPORT std::string GetJemaOSGeolocationAPIUrl();

// Retrieves the Timezone API URL
CHROMEOS_EXPORT std::string GetJemaOSTimezoneAPIUrl();

// Retrieves the Looking Glass URL
CHROMEOS_EXPORT std::string GetJemaOSLookingGlassUrl();

// Retrieves the App Store URL
CHROMEOS_EXPORT std::string GetJemaOSAppStoreURL();

// Retrieves the Web Store Update URL
CHROMEOS_EXPORT std::string GetJemaOSWebStoreUpdateUrl();

// Retrieves the Assistant Web URL
CHROMEOS_EXPORT std::string GetJemaOSAssistantWebUrl();

// Converts the Web Store Update URL based on the domain
CHROMEOS_EXPORT std::string MayConvertWebStoreUpdateUrl(
    const std::string& url);

}  // namespace switches
}  // namespace jemaos

#endif  // CHROMEOS_JEMAOS_SWITCHES_SERVICES_SWITCHES_H_