// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_SERVICES_CONSTANTS_H_
#define CHROMEOS_JEMAOS_SWITCHES_SERVICES_CONSTANTS_H_

#include "chromeos/chromeos_export.h"
#include <string>

namespace jemaos {
namespace constants {

// Default API URLs for JemaOS services
CHROMEOS_EXPORT extern const char kDefaultJemaOSGeolocationAPIUrl[];
CHROMEOS_EXPORT extern const char kDefaultJemaOSTimezoneAPIUrl[];

CHROMEOS_EXPORT extern const char kDefaultJemaOSLookingGlassUrl[];

CHROMEOS_EXPORT extern const char kJemaOSFeedbackPostUrl[];

// Cryptographic key for JemaOS
CHROMEOS_EXPORT extern const uint8_t* kJemaOSCryptoKey;
extern const size_t kJemaOSCryptoKeyLength;

// JemaOS Store App ID
CHROMEOS_EXPORT extern const char kJemaOSStoreAppId[];

// Web Store Update URL for JemaOS
CHROMEOS_EXPORT extern const char kJemaOSWebStoreUpdateURL[];

// Default web URL for JemaOS Assistant
CHROMEOS_EXPORT extern const char kJemaOSAssistantDefaultWebUrl[];

}  // namespace constants
}  // namespace jemaos

#endif  // CHROMEOS_JEMAOS_SWITCHES_SERVICES_CONSTANTS_H_