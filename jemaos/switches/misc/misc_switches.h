// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_MISC_SWITCHES_H_
#define CHROMEOS_JEMAOS_SWITCHES_MISC_SWITCHES_H_

#include <string>
#include "chromeos/chromeos_export.h"

namespace jemaos {
namespace switches {

// Checks if custom JemaOS features are enabled
CHROMEOS_EXPORT bool IsJemaCustomEnabled();

// Checks if TPM dictionary attack lockout is ignored
CHROMEOS_EXPORT bool IsTpmDictionaryAttackLockoutIgnored();

// Checks if the given board is a "Non For You" board
CHROMEOS_EXPORT bool IsNonForYouBoard(const std::string& board);

// Checks if initializing device policy without state keys is allowed
CHROMEOS_EXPORT bool IsInitDevicePolicyWithoutStateKeysAllowed();

// Checks if notifications for unknown peripheral batteries are disabled
CHROMEOS_EXPORT bool IsUnknownPeripheralBatteryNotificationDisabled();

// Checks if dynamic default wallpaper is supported
CHROMEOS_EXPORT bool IsDynamicDefaultWallpaperSupported();

// Command-line switch for testing service host suffix
CHROMEOS_EXPORT extern const char kJemaOSServiceHostSuffixForTesting[];

}  // namespace switches
}  // namespace jemaos

#endif  // CHROMEOS_JEMAOS_SWITCHES_MISC_SWITCHES_H_