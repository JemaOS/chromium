// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * ⚠️ ⚠️ ⚠️  IMPORTANT NOTICE FOR DEVELOPERS  ⚠️ ⚠️ ⚠️
 * 
 * ALL URLS DEFINED IN THIS FILE ARE EXAMPLE PLACEHOLDERS ONLY!
 * 
 * These URLs are NOT functional and will NOT work in production.
 * They are provided as examples to show the expected URL structure
 * and naming conventions.
 * 
 * BEFORE DEPLOYING TO PRODUCTION, YOU MUST:
 * 1. Replace ALL example URLs with real, working API endpoints
 * 2. Ensure all APIs are properly implemented and tested
 * 3. Verify SSL certificates and security configurations
 * 4. Update documentation with the actual API specifications
 * 
 * Key services that need real implementations:
 * - Geolocation API (IP-based location detection)
 * - Timezone API (automatic timezone configuration)
 * - Looking Glass (system diagnostics and telemetry)
 * - Web Store (application updates and downloads)
 * - Feedback API (user feedback and bug reporting)
 * - AI Assistant (intelligent user assistance)
 */

#include "jemaos/switches/services/services_constants.h"
#include "jemaos/build/config/buildflags.h"
#include "jemaos/switches/account/policy_constants.h"

namespace jemaos::constants {

#if BUILDFLAG(USE_JEMAOS_COM)
// URLs for JemaOS services when using the .com domain
// ⚠️  IMPORTANT: These URLs are EXAMPLE PLACEHOLDERS ONLY and must be replaced 
// with real, functional API endpoints before production deployment.
// These example URLs will NOT work and are provided for development/testing purposes.
const char kDefaultJemaOSGeolocationAPIUrl[] = "https://apis.jemaos.com/geo/locationByIp?";  // EXAMPLE - Replace with real geolocation API
const char kDefaultJemaOSTimezoneAPIUrl[] = "https://apis.jemaos.com/geo/timezone?";        // EXAMPLE - Replace with real timezone API
const char kDefaultJemaOSLookingGlassUrl[] = "https://lookingglass.jemaos.com";            // EXAMPLE - Replace with real diagnostics service
const char kJemaOSWebStoreUpdateURL[] = "https://store.jemaos.com";                        // EXAMPLE - Replace with real app store URL
const char kJemaOSFeedbackPostUrl[] = "https://apis.jemaos.com/feedback/submit";           // EXAMPLE - Replace with real feedback API
#else
// URLs for JemaOS services when using the .io domain
// ⚠️  IMPORTANT: These URLs are EXAMPLE PLACEHOLDERS ONLY and must be replaced 
// with real, functional API endpoints before production deployment.
// These example URLs will NOT work and are provided for development/testing purposes.
const char kDefaultJemaOSGeolocationAPIUrl[] = "https://apis.jemaos.io/geo/locationByIp?";  // EXAMPLE - Replace with real geolocation API
const char kDefaultJemaOSTimezoneAPIUrl[] = "https://apis.jemaos.io/geo/timezone?";        // EXAMPLE - Replace with real timezone API
const char kDefaultJemaOSLookingGlassUrl[] = "https://lookingglass.jemaos.io";            // EXAMPLE - Replace with real diagnostics service
const char kJemaOSWebStoreUpdateURL[] = "https://store.jemaos.io";                        // EXAMPLE - Replace with real app store URL
const char kJemaOSFeedbackPostUrl[] = "https://apis.jemaos.io/feedback/submit";           // EXAMPLE - Replace with real feedback API
#endif

// Cryptographic key for JemaOS
const uint8_t* kJemaOSCryptoKey = jemaos::constants::kJemaOSPolicyVerificationKey;
const size_t kJemaOSCryptoKeyLength = jemaos::constants::kJemaOSPolicyVerificationKeyLength;

// JemaOS Store App ID
const char kJemaOSStoreAppId[] = "hidnajblbifdkmheebalalchohohmaef";

// Default web URL for JemaOS Assistant
// ⚠️  EXAMPLE URL - Replace with real AI assistant service endpoint
const char kJemaOSAssistantDefaultWebUrl[] = "https://aia.jemaos.io/";  // EXAMPLE - Replace with real assistant URL

}  // namespace jemaos::constants
