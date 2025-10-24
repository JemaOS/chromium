// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_URLS_CONSTANTS_
#define CHROMEOS_JEMAOS_SWITCHES_URLS_CONSTANTS_

#include <string>
#include "chromeos/chromeos_export.h"
#include "jemaos/build/config/buildflags.h"

namespace jemaos::constants {

CHROMEOS_EXPORT extern const char kDefaultTestUrl[];
CHROMEOS_EXPORT extern const char kJemaOSHomePageUrl[];
CHROMEOS_EXPORT extern const char kOpenJemaHomePageUrl[];

CHROMEOS_EXPORT extern const char kJemaOSStoreBaseUrl[];
CHROMEOS_EXPORT extern const char kJemaOSAccountBaseUrl[];

extern const char kJemaOSForumURL[];
extern const char kJemaOSRemoteDesktopURL[];
extern const char kJemaOSNotesAppURL[];
extern const char kJemaOSHelpURL[];
extern const char kJemaOSReleaseNotesURL[];
extern const char kJemaOSNewsURL[];
extern const char kMultiDeviceLearnMoreURL[];
extern const char kLanguageSettingsLearnMoreUrl[];
extern const char kLinuxAppsLearnMoreURL[];
extern const char kOnlineEulaURLPath[];
extern const char kOnlinePrivacyURLPath[];
extern const char kEulaURLPath[];
extern const char kPrivacyURLPath[];
extern const char kJemaAccessibilityHelpURL[];
extern const char kJemaNewGestureHelpURL[];
extern const char kSmbSharesLearnMoreURL[];
extern const char kCupsPrintLearnMoreURL[];
extern const char kNaturalScrollHelpURL[];
extern const char kControlledScrollingHelpURL[];
extern const char kTimeZoneSettingsLearnMoreURL[];
extern const char kResetProfileSettingsLearnMoreURL[];
extern const char kCrosBatterySaverLearnMoreURL[];

CHROMEOS_EXPORT extern const char kJemaOSAccountURL[];
CHROMEOS_EXPORT extern const char kJemaOSAccountChooserURL[];
CHROMEOS_EXPORT extern const char kJemaOSPasswordManagerURL[];

CHROMEOS_EXPORT extern const char kGoogleDriveBuyStorageUrl[];
CHROMEOS_EXPORT extern const char kGoogleDriveOverviewUrl[];
CHROMEOS_EXPORT extern const char kGoogleDriveHelpUrl[];
CHROMEOS_EXPORT extern const char kGoogleDriveOfflineHelpUrl[];
CHROMEOS_EXPORT extern const char kGoogleDriveRootUrl[];
CHROMEOS_EXPORT extern const char kHelpURLFormat[];
CHROMEOS_EXPORT extern const char kHelpURLNoTaskForFile[];
CHROMEOS_EXPORT extern const char kJemaDropUrl[];

CHROMEOS_EXPORT extern const char kJemaExperimentTpmFallbackUrl[];

extern const char kEolNotificationURL[];
extern const char kKeyboardShortcutHelpPageUrl[];
extern const char kJemaOSEnableWidevineLearnMoreURL[];

extern const char kWifiHiddenNetworkURL[];
extern const char kBluetoothPairingLearnMoreUrl[];
extern const char kFileManagerHelpURL[];
extern const char kTabletModeGesturesLearnMoreURL[];
extern const char kRuntimeHostPermissionsHelpURL[];
extern const char kFingerprintLearnMoreURL[];

extern const char kJemaOSBackupRestoreLearnMoreURL[];

extern const char kJemaOSToggleArcMediaAutoScanLearnMoreURL[];

extern const char kJemaOSDevModeTransitionLearnMoreURL[];

#if BUILDFLAG(JEMAOS_DEVICE)
extern const char kJemaOSProductWarrantyDefaultURL[];
#endif

#if !BUILDFLAG(USE_JEMAOS_COM)
extern const char kJemaOSDiscordServerURL[];
extern const char kJemaOSTelegramGroupURL[];
#endif
}  // namespace jemaos::constants

#endif
