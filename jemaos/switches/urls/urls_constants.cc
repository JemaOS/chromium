// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/urls/urls_constants.h"
#include "jemaos/build/config/buildflags.h"

namespace jemaos::constants {

#if BUILDFLAG(USE_JEMAOS_COM)

const char kDefaultTestUrl[] = "http://store.jemaos.com/204";
const char kJemaOSHomePageUrl[] = "https://jemaos.com";

const char kJemaOSStoreBaseUrl[] = "https://store.jemaos.com";
const char kJemaOSAccountBaseUrl[] = "https://account.jemaos.com";

const char kJemaOSForumURL[] = "https://community.jemaos.com/";
const char kJemaOSRemoteDesktopURL[] = "https://rdp.jemaos.com/";
const char kJemaOSHelpURL[] = "https://jemaos.com/help/";
const char kJemaOSReleaseNotesURL[] = "https://jemaos.com/release";
const char kMultiDeviceLearnMoreURL[] = "https://jemaos.com/docs";
const char kLanguageSettingsLearnMoreUrl[] = "https://jemaos.com/docs/manual/customize-settings/language/manage-your-jemaos-devices-languages/";
const char kLinuxAppsLearnMoreURL[] = "https://jemaos.com/docs/manual/manage-your-apps/add-apps-and-extensions/set-up-linux-on-your-jemaos-device/";
const char kOnlineEulaURLPath[] = "https://jemaos.com/terms-of-service/?agent=oobe";
const char kOnlinePrivacyURLPath[] = "https://jemaos.com/privacy/?agent=oobe";
const char kJemaAccessibilityHelpURL[] = "https://jemaos.com/docs/manual/explore-accessibility";
const char kJemaNewGestureHelpURL[] = "https://jemaos.com/docs/manual/customize-settings/use-tablets/use-gestures-or-buttons-to-navigate-in-tablet-mode";
const char kSmbSharesLearnMoreURL[] = "https://jemaos.com/docs";
const char kCupsPrintLearnMoreURL[] = "https://jemaos.com/docs";
const char kNaturalScrollHelpURL[] = "https://jemaos.com/docs/manual/customize-settings/appearance/use-your-jemaos-device-touchpad/";
const char kJemaOSAccountURL[] = "https://account.jemaos.com";
const char kJemaOSAccountChooserURL[] = "https://account.jemaos.com";
const char kJemaOSPasswordManagerURL[] = "https://account.jemaos.com";

const char kGoogleDriveBuyStorageUrl[] = "https://jemaos.com/docs/knowledge-base";
const char kGoogleDriveOverviewUrl[] = "https://jemaos.com/docs/knowledge-base";
const char kGoogleDriveHelpUrl[] = "https://jemaos.com/docs/knowledge-base";
const char kGoogleDriveOfflineHelpUrl[] = "https://jemaos.com/docs/knowledge-base";
const char kGoogleDriveRootUrl[] = "https://jemaos.com";
const char kHelpURLFormat[] = "https://jemaos.com/docs/knowledge-base/answer/%d";
const char kHelpURLNoTaskForFile[] = "https://jemaos.com/docs/manual/manage-your-apps/files-and-downloads/file-types-and-external-devices-that-work-on-jemaos-device/";
const char kJemaDropUrl[] = "https://snapdrop.net/";
const char kJemaDiagnosticAppUrl[] = "https://jemaos.com/docs/manual/fix-problems/use-diagnostics-to-troubleshoot";
const char kJemaExperimentTpmFallbackUrl[] = "https://jemaos.com/faq/experimental-tpm-fallback";
const char kEolNotificationURL[] = "https://jemaos.com/content/eol";
const char kKeyboardShortcutHelpPageUrl[] = "https://jemaos.com/docs/knowledge-base/recipes/keyboard-shortcuts";
const char kJemaOSEnableWidevineLearnMoreURL[] = "https://jemaos.com/docs/knowledge-base/recipes/widevine";
const char kWifiHiddenNetworkURL[] = "https://jemaos.com/docs/manual/connect-your-jemaos-device/connect-to-wi-fi-&-other-networks/manage-wifi-networks/";
const char kBluetoothPairingLearnMoreUrl[] = "https://jemaos.com/docs/manual/connect-your-jemaos-device/connect-to-other-devices/connect-to-bluetooth-devices/";
const char kFileManagerHelpURL[] = "https://jemaos.com/docs/manual/manage-your-apps/files-and-downloads/open-save-or-delete-files/";
const char kTabletModeGesturesLearnMoreURL[] = "https://jemaos.com/docs/manual/customize-settings/use-tablets/use-gestures-or-buttons-to-navigate-in-tablet-mode";
const char kRuntimeHostPermissionsHelpURL[] = "https://jemaos.com/docs/manual/manage-your-apps/add-apps-and-extensions/add-apps-and-extensions/";
const char kFingerprintLearnMoreURL[] = "https://jemaos.com/docs/manual/customize-settings/users-and-sync/set-up-and-sign-in-with-fingerprint-on-your-jemaos-device";

const char kJemaOSBackupRestoreLearnMoreURL[] = "https://jemaos.com/docs/manual/customize-settings/jemaos-settings/misc#backup-and-restore";

#else

const char kDefaultTestUrl[] = "http://store.jemaos.io/204";
const char kJemaOSHomePageUrl[] = "https://jemaos.io";

const char kJemaOSStoreBaseUrl[] = "https://store.jemaos.io";
const char kJemaOSAccountBaseUrl[] = "https://account.jemaos.io";

const char kJemaOSForumURL[] = "https://community.jemaos.io/";
const char kJemaOSRemoteDesktopURL[] = "https://rdp.jemaos.io/";
const char kJemaOSHelpURL[] = "https://jemaos.io/help/";
const char kJemaOSReleaseNotesURL[] = "https://jemaos.io/release";
const char kMultiDeviceLearnMoreURL[] = "https://jemaos.io/docs";
const char kLanguageSettingsLearnMoreUrl[] = "https://jemaos.io/docs/manual/customize-settings/language";
const char kLinuxAppsLearnMoreURL[] = "https://jemaos.io/docs/manual/manage-your-apps/add-apps-and-extensions/set-up-linux-on-your-jemaos-device/";
const char kOnlineEulaURLPath[] = "https://jemaos.io/terms-of-service/?agent=oobe";
const char kOnlinePrivacyURLPath[] = "https://jemaos.io/privacy/?agent=oobe";
const char kJemaAccessibilityHelpURL[] = "https://jemaos.io/docs/manual/explore-accessibility";
const char kJemaNewGestureHelpURL[] = "https://jemaos.io/docs/manual/customize-settings/use-tablets/use-gestures-or-buttons-to-navigate-in-tablet-mode";
const char kSmbSharesLearnMoreURL[] = "https://jemaos.io/docs";
const char kCupsPrintLearnMoreURL[] = "https://jemaos.io/docs";
const char kNaturalScrollHelpURL[] = "https://jemaos.io/docs/manual/customize-settings/appearance/use-your-jemaos-device-touchpad/";
const char kJemaOSAccountURL[] = "https://account.jemaos.io";
const char kJemaOSAccountChooserURL[] = "https://account.jemaos.io";
const char kJemaOSPasswordManagerURL[] = "https://account.jemaos.io";

const char kGoogleDriveBuyStorageUrl[] = "https://jemaos.io/docs/knowledge-base";
const char kGoogleDriveOverviewUrl[] = "https://jemaos.io/docs/knowledge-base";
const char kGoogleDriveHelpUrl[] = "https://jemaos.io/docs/knowledge-base";
const char kGoogleDriveOfflineHelpUrl[] = "https://jemaos.io/docs/knowledge-base";
const char kGoogleDriveRootUrl[] = "https://jemaos.io";
const char kHelpURLFormat[] = "https://jemaos.io/docs/knowledge-base/answer/%d";
const char kHelpURLNoTaskForFile[] = "https://jemaos.io/docs/manual/manage-your-apps/files-and-downloads/file-types-and-external-devices-that-work-on-jemaos-device/";
const char kJemaDropUrl[] = "https://snapdrop.net/";
const char kJemaDiagnosticAppUrl[] = "https://jemaos.io/docs/manual/fix-problems/use-diagnostics-to-troubleshoot";
const char kJemaExperimentTpmFallbackUrl[] = "https://jemaos.io/faq/experimental-tpm-fallback";
const char kEolNotificationURL[] = "https://jemaos.io/content/eol";
const char kKeyboardShortcutHelpPageUrl[] = "https://jemaos.io/docs/knowledge-base/recipes/keyboard-shortcuts";
const char kJemaOSEnableWidevineLearnMoreURL[] = "https://jemaos.io/docs/knowledge-base/recipes/widevine";
const char kWifiHiddenNetworkURL[] = "https://jemaos.io/docs/manual/connect-your-jemaos-device/connect-to-wi-fi-&-other-networks/manage-wifi-networks/";
const char kBluetoothPairingLearnMoreUrl[] = "https://jemaos.io/docs/manual/connect-your-jemaos-device/connect-to-other-devices/connect-to-bluetooth-devices/";
const char kFileManagerHelpURL[] = "https://jemaos.io/docs/manual/manage-your-apps/files-and-downloads/open-save-or-delete-files/";
const char kTabletModeGesturesLearnMoreURL[] = "https://jemaos.io/docs/manual/customize-settings/use-tablets/use-gestures-or-buttons-to-navigate-in-tablet-mode";
const char kRuntimeHostPermissionsHelpURL[] = "https://jemaos.io/docs/manual/manage-your-apps/add-apps-and-extensions/add-apps-and-extensions/";
const char kFingerprintLearnMoreURL[] = "https://jemaos.io/docs/manual/customize-settings/users-and-sync/set-up-and-sign-in-with-fingerprint-on-your-jemaos-device";

const char kJemaOSBackupRestoreLearnMoreURL[] = "https://jemaos.io/docs/manual/customize-settings/jemaos-settings/misc#backup-and-restore";

#endif

}  // jemaos::constants
