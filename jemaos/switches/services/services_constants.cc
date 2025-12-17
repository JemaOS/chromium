// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/services/services_constants.h"
#include "jemaos/build/config/buildflags.h"
#include "jemaos/switches/account/policy_constants.h"

namespace jemaos::constants {

// ⚠️ IMPORTANT: These URLs are EXAMPLE PLACEHOLDERS ONLY and must be replaced
// with real, functional API endpoints before production deployment.
// These example URLs will NOT work and are provided for development/testing purposes.
#if BUILDFLAG(USE_JEMAOS_COM)
const char kDefaultJemaOSGeolocationAPIUrl[] =
  "https://l8rof5h3z7.execute-api.us-east-1.amazonaws.com/geo/locationByIp?"; // EXAMPLE - Replace with real geolocation API
const char kDefaultJemaOSTimezoneAPIUrl[] =
  "https://l8rof5h3z7.execute-api.us-east-1.amazonaws.com/geo/timezone?"; // EXAMPLE - Replace with real timezone API
const char kDefaultJemaOSLookingGlassUrl[] =
  "https://l8rof5h3z7.execute-api.us-east-1.amazonaws.com/lookingglass"; // EXAMPLE - Replace with real diagnostics service
const char kJemaOSWebStoreUpdateURL[] =
  "https://l8rof5h3z7.execute-api.us-east-1.amazonaws.com/store"; // EXAMPLE - Replace with real app store URL
const char kJemaOSFeedbackPostUrl[] =
  "https://l8rof5h3z7.execute-api.us-east-1.amazonaws.com/feedback/submit"; // EXAMPLE - Replace with real feedback API

const char kJemaOSAssistantDefaultWebUrl[] = "https://ai.jemaos.com/chat/";
#else
// URLs for JemaOS services when using the .io domain
// ⚠️ IMPORTANT: These URLs are EXAMPLE PLACEHOLDERS ONLY and must be replaced
// with real, functional API endpoints before production deployment.
// These example URLs will NOT work and are provided for development/testing purposes.
const char kDefaultJemaOSGeolocationAPIUrl[] =
  "https://l8rof5h3z7.execute-api.us-east-1.amazonaws.com/geo/locationByIp?"; // EXAMPLE - Replace with real geolocation API
const char kDefaultJemaOSTimezoneAPIUrl[] =
  "https://l8rof5h3z7.execute-api.us-east-1.amazonaws.com/geo/timezone?"; // EXAMPLE - Replace with real timezone API
const char kDefaultJemaOSLookingGlassUrl[] =
  "https://l8rof5h3z7.execute-api.us-east-1.amazonaws.com/lookingglass"; // EXAMPLE - Replace with real diagnostics service
const char kJemaOSWebStoreUpdateURL[] =
  "https://l8rof5h3z7.execute-api.us-east-1.amazonaws.com/store"; // EXAMPLE - Replace with real app store URL
const char kJemaOSFeedbackPostUrl[] =
  "https://l8rof5h3z7.execute-api.us-east-1.amazonaws.com/feedback/submit"; // EXAMPLE - Replace with real feedback API

const char kJemaOSAssistantDefaultWebUrl[] = "https://ai.jematechnology.fr/chat/";
#endif

const uint8_t* kJemaOSCryptoKey =
  jemaos::constants::kJemaOSPolicyVerificationKey;
const size_t kJemaOSCryptoKeyLength =
  jemaos::constants::kJemaOSPolicyVerificationKeyLength;

const char kJemaOSStoreAppId[] = "hidnajblbifdkmheebalalchohohmaef";

}  // namespace jemaos::constants
