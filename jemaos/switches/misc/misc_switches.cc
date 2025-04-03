// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/misc/misc_switches.h"
#include "base/command_line.h"
#include "base/strings/string_util.h"

namespace jemaos {
namespace switches {

namespace {

// Command-line switch to disable custom JemaOS features
const char kJemaDisableCustom[] = "jema-disable-custom";

// Command-line switch to enable TPM dictionary attack lockout
const char kEnableTpmDictionaryAttackLockout[] = "jemaos-enable-tpm-da-lockout";

// Command-line switch to allow initializing device policy without state keys
const char kAllowInitDevicePolicyWithoutStateKeys[] = "allow-init-device-policy-without-state-keys";

// Command-line switch to disable notifications for unknown peripheral batteries
const char kDisableUnknownPeripheralBatteryNotification[] = "disable-unknown-peripheral-battery-notification";

// Command-line switch to enable dynamic default wallpaper
const char kJemaEnableDynamicDefaultWallpaper[] = "jema-dynamic-default-wallpaper";

// List of boards that are not supported for "For You" features
const std::vector<std::string> kNonForYouBoards = {
  "amd64-jemaos",
  "amd64-openjema",
  "amd64-vmware",
  "amd64-generic",
};

}  // namespace

// Command-line switch for testing service host suffix
const char kJemaOSServiceHostSuffixForTesting[] = "jemaos-service-host-suffix-for-testing";

// Checks if custom JemaOS features are enabled
bool IsJemaCustomEnabled() {
  return !base::CommandLine::ForCurrentProcess()->HasSwitch(kJemaDisableCustom);
}

// Checks if TPM dictionary attack lockout is ignored
bool IsTpmDictionaryAttackLockoutIgnored() {
  return !base::CommandLine::ForCurrentProcess()->HasSwitch(kEnableTpmDictionaryAttackLockout);
}

// Checks if the given board is a "Non For You" board
bool IsNonForYouBoard(const std::string& board) {
  for (const auto& non_for_you_board : kNonForYouBoards) {
    if (non_for_you_board == board) {
      return true;
    }
    if (base::StartsWith(board, non_for_you_board)) {
      return true;
    }
  }
  return false;
}

// Checks if initializing device policy without state keys is allowed
bool IsInitDevicePolicyWithoutStateKeysAllowed() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kAllowInitDevicePolicyWithoutStateKeys);
}

// Checks if notifications for unknown peripheral batteries are disabled
bool IsUnknownPeripheralBatteryNotificationDisabled() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kDisableUnknownPeripheralBatteryNotification);
}

// Checks if dynamic default wallpaper is supported
bool IsDynamicDefaultWallpaperSupported() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kJemaEnableDynamicDefaultWallpaper);
}

}  // namespace switches
}  // namespace jemaos