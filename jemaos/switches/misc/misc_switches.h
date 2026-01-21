// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_MISC_SWITCHES_H_
#define CHROMEOS_JEMAOS_SWITCHES_MISC_SWITCHES_H_

#include <string>
#include "chromeos/chromeos_export.h"

namespace jemaos {
namespace switches {

CHROMEOS_EXPORT bool IsJemaCustomEnabled();

CHROMEOS_EXPORT bool IsTpmDictionaryAttackLockoutIgnored();

CHROMEOS_EXPORT bool IsNonForYouBoard(const std::string& board);

CHROMEOS_EXPORT bool IsInitDevicePolicyWithoutStateKeysAllowed();

CHROMEOS_EXPORT bool IsUnknownPeripheralBatteryNotificationDisabled();

CHROMEOS_EXPORT bool IsDynamicDefaultWallpaperSupported();

CHROMEOS_EXPORT extern const char kJemaOSServiceHostSuffixForTesting[];

CHROMEOS_EXPORT int64_t GetJemaOSAutoSigninDelay();

CHROMEOS_EXPORT bool IsOsInstallButtonHidden();

// Returns true when the Google account login button should be hidden in the
// OOBE / welcome flow (account type selection).
CHROMEOS_EXPORT bool IsGoogleLoginButtonHidden();

CHROMEOS_EXPORT bool IsJemaAiHidden();

} // switches
} // jemaos

#endif
