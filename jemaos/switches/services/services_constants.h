// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_SERVICES_CONSTANTS_H_
#define CHROMEOS_JEMAOS_SWITCHES_SERVICES_CONSTANTS_H_

#include "chromeos/chromeos_export.h"
#include <string>

namespace jemaos {
namespace constants {

CHROMEOS_EXPORT extern const char kDefaultJemaOSGeolocationAPIUrl[];
CHROMEOS_EXPORT extern const char kDefaultJemaOSTimezoneAPIUrl[];

CHROMEOS_EXPORT extern const char kDefaultJemaOSLookingGlassUrl[];

CHROMEOS_EXPORT extern const char kJemaOSFeedbackPostUrl[];

CHROMEOS_EXPORT extern const uint8_t* kJemaOSCryptoKey;
extern const size_t kJemaOSCryptoKeyLength;

extern const char kJemaOSStoreAppId[];

CHROMEOS_EXPORT extern const char kJemaOSWebStoreUpdateURL[];

CHROMEOS_EXPORT extern const char kJemaOSAssistantDefaultWebUrl[];
} // constants
} // jemaos

#endif
