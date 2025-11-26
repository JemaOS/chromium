// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/misc/misc_switches.h"
#include "base/command_line.h"
#include "base/strings/string_util.h"

namespace jemaos {
namespace switches {

namespace {

const char kJemaDisableCustom[] = "jema-disable-custom";

const char kEnableTpmDictionaryAttackLockout[] = "jemaos-enable-tpm-da-lockout";

const char kDisallowInitDevicePolicyWithoutStateKeys[] = "disallow-init-device-policy-without-state-keys";

const char kDisableUnknownPeripheralBatteryNotification[] = "disable-unknown-peripheral-battery-notification";

const char kJemaEnableDynamicDefaultWallpaper[] =
  "jema-dynamic-default-wallpaper";

const char kJemaHideOsInstallButton[] = "jema-hide-os-install-button";

const std::vector<std::string> kNonForYouBoards = {
  "amd64-jemaos",
  "amd64-openjema",
  "amd64-vmware",
  "amd64-generic",
  "jematab_duo-jemaos",
};

const char kJemaAutoSigninDelay[] = "jema-auto-signin-delay";

}

const char kJemaOSServiceHostSuffixForTesting[] =
  "jemaos-service-host-suffix-for-testing";

bool IsJemaCustomEnabled() {
  return !base::CommandLine::ForCurrentProcess()->HasSwitch(kJemaDisableCustom);
}

bool IsTpmDictionaryAttackLockoutIgnored() {
  return !base::CommandLine::ForCurrentProcess()->HasSwitch(kEnableTpmDictionaryAttackLockout);
}

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

bool IsInitDevicePolicyWithoutStateKeysAllowed() {
  return !base::CommandLine::ForCurrentProcess()->HasSwitch(kDisallowInitDevicePolicyWithoutStateKeys);
}

bool IsUnknownPeripheralBatteryNotificationDisabled() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kDisableUnknownPeripheralBatteryNotification);
}

bool IsDynamicDefaultWallpaperSupported() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(
      kJemaEnableDynamicDefaultWallpaper);
}

int64_t GetJemaOSAutoSigninDelay() {
	std::string delayStr = base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(kJemaAutoSigninDelay);
	if (delayStr.empty())
		return 0;
	return (int64_t) std::stoi(delayStr);
}

bool IsOsInstallButtonHidden() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kJemaHideOsInstallButton);
}

} // switches
} // jemaos
