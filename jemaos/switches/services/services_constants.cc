// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/services/services_constants.h"
#include "jemaos/build/config/buildflags.h"
#include "jemaos/switches/account/policy_constants.h"

namespace jemaos::constants {

#if BUILDFLAG(USE_JEMAOS_COM)
const char kDefaultJemaOSGeolocationAPIUrl[] =
  "https://apis.jemaos.com/geo/locationByIp?";
const char kDefaultJemaOSTimezoneAPIUrl[] =
  "https://apis.jemaos.com/geo/timezone?";
const char kDefaultJemaOSLookingGlassUrl[] =
  "https://apis.jemaos.com/lookingglass";
const char kJemaOSWebStoreUpdateURL[] = "https://store.jemaos.com";
const char kJemaOSFeedbackPostUrl[] = "https://apis.jemaos.com/feedback/submit";

const char kJemaOSAssistantDefaultWebUrl[] = "https://ai.jemaos.com/chat/";
#else
const char kDefaultJemaOSGeolocationAPIUrl[] =
  "https://apis.jemaos.io/geo/locationByIp?";
const char kDefaultJemaOSTimezoneAPIUrl[] =
  "https://apis.jemaos.io/geo/timezone?";
const char kDefaultJemaOSLookingGlassUrl[] =
  "https://apis.jemaos.io/lookingglass";
const char kJemaOSWebStoreUpdateURL[] = "https://store.jemaos.io";
const char kJemaOSFeedbackPostUrl[] = "https://apis.jemaos.io/feedback/submit";

const char kJemaOSAssistantDefaultWebUrl[] = "https://ai.jemaos.io/chat/";
#endif

const uint8_t* kJemaOSCryptoKey =
  jemaos::constants::kJemaOSPolicyVerificationKey;
const size_t kJemaOSCryptoKeyLength =
  jemaos::constants::kJemaOSPolicyVerificationKeyLength;

const char kJemaOSStoreAppId[] = "hidnajblbifdkmheebalalchohohmaef";

}  // namespace jemaos::constants
