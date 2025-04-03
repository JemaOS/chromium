// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/services/services_constants.h"
#include "jemaos/build/config/buildflags.h"
#include "jemaos/switches/account/policy_constants.h"

namespace jemaos::constants {

#if BUILDFLAG(USE_JEMAOS_COM)
// URLs for JemaOS services when using the .com domain
// NOTE: These URLs are placeholders and will need to be updated to the correct API endpoints.
const char kDefaultJemaOSGeolocationAPIUrl[] = "https://apis.jemaos.com/geo/locationByIp?";
const char kDefaultJemaOSTimezoneAPIUrl[] = "https://apis.jemaos.com/geo/timezone?";
const char kDefaultJemaOSLookingGlassUrl[] = "https://lookingglass.jemaos.com";
const char kJemaOSWebStoreUpdateURL[] = "https://store.jemaos.com";
const char kJemaOSFeedbackPostUrl[] = "https://apis.jemaos.com/feedback/submit";
#else
// URLs for JemaOS services when using the .io domain
// NOTE: These URLs are placeholders and will need to be updated to the correct API endpoints.
const char kDefaultJemaOSGeolocationAPIUrl[] = "https://apis.jemaos.io/geo/locationByIp?";
const char kDefaultJemaOSTimezoneAPIUrl[] = "https://apis.jemaos.io/geo/timezone?";
const char kDefaultJemaOSLookingGlassUrl[] = "https://lookingglass.jemaos.io";
const char kJemaOSWebStoreUpdateURL[] = "https://store.jemaos.io";
const char kJemaOSFeedbackPostUrl[] = "https://apis.jemaos.io/feedback/submit";
#endif

// Cryptographic key for JemaOS
const uint8_t* kJemaOSCryptoKey = jemaos::constants::kJemaOSPolicyVerificationKey;
const size_t kJemaOSCryptoKeyLength = jemaos::constants::kJemaOSPolicyVerificationKeyLength;

// JemaOS Store App ID
const char kJemaOSStoreAppId[] = "hidnajblbifdkmheebalalchohohmaef";

// Default web URL for JemaOS Assistant
const char kJemaOSAssistantDefaultWebUrl[] = "https://aia.jemaos.io/";

}  // namespace jemaos::constants