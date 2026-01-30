// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/urls/urls_constants.h"
#include "jemaos/build/config/buildflags.h"

namespace jemaos::constants {

#if BUILDFLAG(USE_JEMAOS_COM)

const char kDefaultTestUrl[] = "http://jematech.fr/204";
const char kJemaOSHomePageUrl[] = "https://jematechnology.fr";
const char kOpenJemaHomePageUrl[] = "https://jematechnology.fr";

const char kJemaOSStoreBaseUrl[] = "https://chromewebstore.google.com/category/extensions";
const char kJemaOSAccountBaseUrl[] = "https://staging.d3ihq4vq2ptyl6.amplifyapp.com";

const char kJemaOSForumURL[] = "https://jematechnology.fr/";
const char kJemaOSRemoteDesktopURL[] = "https://jematechnology.fr/"; // https://rdp.jematechnology.fr/
const char kJemaOSNotesAppURL[] = "https://jematechnology.fr"; // https://notes.jematechnology.fr
const char kJemaOSHelpURL[] = "https://jematechnology.fr/?ospath=help/";
const char kJemaOSReleaseNotesURL[] = "https://jematechnology.fr/?ospath=release";
const char kJemaOSNewsURL[] = "https://jematechnology.fr/?ospath=blog";
const char kMultiDeviceLearnMoreURL[] = "https://jematechnology.fr/?ospath=docs";
const char kLanguageSettingsLearnMoreUrl[] = "https://jematechnology.fr/?ospath=docs/manual/customize-settings/language/manage-your-jemaos-devices-languages/";
const char kLinuxAppsLearnMoreURL[] = "https://jematechnology.fr/?ospath=docs/manual/manage-your-apps/add-apps-and-extensions/set-up-linux-on-your-jemaos-device/";
const char kOnlineEulaURLPath[] = "https://www.jematechnology.fr/conditions-utilisation/?agent=oobe";
const char kOnlinePrivacyURLPath[] = "https://www.jematechnology.fr/politique-confidentialite/?agent=oobe";
const char kEulaURLPath[] = "https://www.jematechnology.fr/conditions-utilisation";
const char kPrivacyURLPath[] = "https://www.jematechnology.fr/conditions-utilisation";
const char kJemaAccessibilityHelpURL[] = "https://jematechnology.fr/?ospath=docs/manual/explore-accessibility";
const char kJemaNewGestureHelpURL[] = "https://jematechnology.fr/?ospath=docs/manual/customize-settings/use-tablets/use-gestures-or-buttons-to-navigate-in-tablet-mode";
const char kSmbSharesLearnMoreURL[] = "https://jematechnology.fr/?ospath=help/manual/manage-your-apps/files-and-downloads/add-network-file-share-system/";
const char kCupsPrintLearnMoreURL[] = "https://jematechnology.fr/?ospath=docs/manual/connect-your-jemaos-device/connect-to-other-devices/set-up-your-printer";
const char kNaturalScrollHelpURL[] = "https://jematechnology.fr/?ospath=docs/manual/customize-settings/appearance/use-your-jemaos-device-touchpad/";
const char kControlledScrollingHelpURL[] = "https://jematechnology.fr/?ospath=docs/manual/customize-settings/appearance/use-your-jemaos-device-touchpad/";
const char kTimeZoneSettingsLearnMoreURL[] = "https://jematechnology.fr/?ospath=help/manual/customize-settings/appearance/set-the-date-and-time/";
const char kResetProfileSettingsLearnMoreURL[] = "https://jematechnology.fr/?ospath=help/manual/customize-settings/privacy/reset-chromium-settings-to-default/";
const char kCrosBatterySaverLearnMoreURL[] = "https://jematechnology.fr/?ospath=help/manual/customize-settings/advanced-settings/battery-saver/";
const char kJemaOSAccountURL[] = "https://account.jematechnology.fr";
const char kJemaOSAccountChooserURL[] = "https://account.jematechnology.fr";
const char kJemaOSPasswordManagerURL[] = "https://account.jematechnology.fr";

const char kGoogleDriveBuyStorageUrl[] = "https://jematechnology.fr/?ospath=docs/knowledge-base";
const char kGoogleDriveOverviewUrl[] = "https://jematechnology.fr/?ospath=docs/knowledge-base";
const char kGoogleDriveHelpUrl[] = "https://jematechnology.fr/?ospath=docs/knowledge-base";
const char kGoogleDriveOfflineHelpUrl[] = "https://jematechnology.fr/?ospath=docs/knowledge-base";
const char kGoogleDriveRootUrl[] = "https://jematechnology.fr";
const char kHelpURLFormat[] = "https://jematechnology.fr/docs/?ospath=knowledge-base/answer/%d";
const char kHelpURLNoTaskForFile[] = "https://jematechnology.fr/?ospath=docs/manual/manage-your-apps/files-and-downloads/file-types-and-external-devices-that-work-on-jemaos-device/";
const char kJemaDropUrl[] = "https://drop.jematechnology.fr";

const char kJemaExperimentTpmFallbackUrl[] = "https://jematechnology.fr/?ospath=faq/experimental-tpm-fallback";
const char kEolNotificationURL[] = "https://jematechnology.fr/?ospath=eol";
const char kKeyboardShortcutHelpPageUrl[] = "https://jematechnology.fr/?ospath=docs/knowledge-base/recipes/keyboard-shortcuts";
const char kJemaOSEnableWidevineLearnMoreURL[] = "https://jematechnology.fr/?ospath=docs/knowledge-base/recipes/widevine";
const char kWifiHiddenNetworkURL[] = "https://jematechnology.fr/?ospath=docs/manual/connect-your-jemaos-device/connect-to-wi-fi-&-other-networks/manage-wifi-networks/";
const char kBluetoothPairingLearnMoreUrl[] = "https://jematechnology.fr/?ospath=docs/manual/connect-your-jemaos-device/connect-to-other-devices/connect-to-bluetooth-devices/";
const char kFileManagerHelpURL[] = "https://jematechnology.fr/?ospath=docs/manual/manage-your-apps/files-and-downloads/open-save-or-delete-files/";
const char kTabletModeGesturesLearnMoreURL[] = "https://jematechnology.fr/?ospath=docs/manual/customize-settings/use-tablets/use-gestures-or-buttons-to-navigate-in-tablet-mode";
const char kRuntimeHostPermissionsHelpURL[] = "https://jematechnology.fr/?ospath=docs/manual/manage-your-apps/add-apps-and-extensions/add-apps-and-extensions/";
const char kFingerprintLearnMoreURL[] = "https://jematechnology.fr/?ospath=docs/manual/customize-settings/users-and-sync/set-up-and-sign-in-with-fingerprint-on-your-jemaos-device";

const char kJemaOSBackupRestoreLearnMoreURL[] = "https://jematechnology.fr/?ospath=docs/manual/customize-settings/jemaos-settings/misc#backup-and-restore";

const char kJemaOSToggleArcMediaAutoScanLearnMoreURL[] = "https://jematechnology.fr/?ospath=faq/disable-media-files-scan/";
const char kJemaOSDevModeTransitionLearnMoreURL[] = "https://jematechnology.fr/?ospath=docs/knowledge-base/getting-started/developer-mode";

#if BUILDFLAG(JEMAOS_DEVICE)
const char kJemaOSProductWarrantyDefaultURL[] = "https://jematechnology.fr"; // https://sn.jematabduo.cn
#endif

#else

const char kDefaultTestUrl[] = "http://chromewebstore.google.com/204";
const char kJemaOSHomePageUrl[] = "https://jematechnology.fr";
const char kOpenJemaHomePageUrl[] = "https://jematechnology.fr/"; // https://jematechnology.fr/openjema/

// Base URL used to open the extensions store UI.
// NOTE: Keep this without a trailing slash because callers append their own
// paths/query params.
const char kJemaOSStoreBaseUrl[] = "https://chromewebstore.google.com";
const char kJemaOSAccountBaseUrl[] = "https://staging.d3ihq4vq2ptyl6.amplifyapp.com";

const char kJemaOSForumURL[] = "https://jematechnology.fr/"; // https://community.jematechnology.fr/
const char kJemaOSRemoteDesktopURL[] = "https://jematechnology.fr/"; // https://rdp.jematechnology.fr/
const char kJemaOSNotesAppURL[] = "https://jematechnology.fr"; // https://notes.jematechnology.fr
const char kJemaOSHelpURL[] = "https://jematechnology.fr/?ospath=help/";
const char kJemaOSReleaseNotesURL[] = "https://jematechnology.fr/?ospath=release";
const char kJemaOSNewsURL[] = "https://jematechnology.fr/?ospath=blog";
const char kMultiDeviceLearnMoreURL[] = "https://jematechnology.fr/?ospath=docs";
const char kLanguageSettingsLearnMoreUrl[] = "https://jematechnology.fr/?ospath=docs/manual/customize-settings/language/manage-your-jemaos-devices-languages/";
const char kLinuxAppsLearnMoreURL[] = "https://jematechnology.fr/?ospath=docs/manual/manage-your-apps/add-apps-and-extensions/set-up-linux-on-your-jemaos-device/";
const char kOnlineEulaURLPath[] = "https://www.jematechnology.fr/conditions-utilisation/?agent=oobe";
const char kOnlinePrivacyURLPath[] = "https://www.jematechnology.fr/conditions-utilisation/?agent=oobe";
const char kEulaURLPath[] = "https://www.jematechnology.fr/conditions-utilisation";
const char kPrivacyURLPath[] = "https://www.jematechnology.fr/conditions-utilisation";
const char kJemaAccessibilityHelpURL[] = "https://jematechnology.fr/?ospath=docs/manual/explore-accessibility";
const char kJemaNewGestureHelpURL[] = "https://jematechnology.fr/?ospath=docs/manual/customize-settings/use-tablets/use-gestures-or-buttons-to-navigate-in-tablet-mode";
const char kSmbSharesLearnMoreURL[] = "https://jematechnology.fr/?ospath=help/manual/manage-your-apps/files-and-downloads/add-network-file-share-system/";
const char kCupsPrintLearnMoreURL[] = "https://jematechnology.fr/?ospath=docs/manual/connect-your-jemaos-device/connect-to-other-devices/set-up-your-printer";
const char kNaturalScrollHelpURL[] = "https://jematechnology.fr/?ospath=ocs/manual/customize-settings/appearance/use-your-jemaos-device-touchpad/";
const char kControlledScrollingHelpURL[] = "https://jematechnology.fr/?ospath=docs/manual/customize-settings/appearance/use-your-jemaos-device-touchpad/";
const char kTimeZoneSettingsLearnMoreURL[] = "https://jematechnology.fr/?ospath=help/manual/customize-settings/appearance/set-the-date-and-time/";
const char kResetProfileSettingsLearnMoreURL[] = "https://jematechnology.fr/?ospath=help/manual/customize-settings/privacy/reset-chromium-settings-to-default/";
const char kCrosBatterySaverLearnMoreURL[] = "https://jematechnology.fr/?ospath=help/manual/customize-settings/advanced-settings/battery-saver/";
const char kJemaOSAccountURL[] = "https://staging.d3ihq4vq2ptyl6.amplifyapp.com";
const char kJemaOSAccountChooserURL[] = "https://staging.d3ihq4vq2ptyl6.amplifyapp.com";
const char kJemaOSPasswordManagerURL[] = "https://staging.d3ihq4vq2ptyl6.amplifyapp.com";

const char kGoogleDriveBuyStorageUrl[] = "https://jematechnology.fr/?ospath=docs/knowledge-base";
const char kGoogleDriveOverviewUrl[] = "https://jematechnology.fr/?ospath=docs/knowledge-base";
const char kGoogleDriveHelpUrl[] = "https://jematechnology.fr/?ospath=docs/knowledge-base";
const char kGoogleDriveOfflineHelpUrl[] = "https://jematechnology.fr/?ospath=docs/knowledge-base";
const char kGoogleDriveRootUrl[] = "https://jematechnology.fr";
const char kHelpURLFormat[] = "https://jematechnology.fr/?ospath=docs/knowledge-base/answer/%d";
const char kHelpURLNoTaskForFile[] = "https://jematechnology.fr/?ospath=docs/manual/manage-your-apps/files-and-downloads/file-types-and-external-devices-that-work-on-jemaos-device/";
const char kJemaDropUrl[] = "https://master.d3tump1ibzy1gt.amplifyapp.com";

const char kJemaExperimentTpmFallbackUrl[] = "https://jematechnology.fr/?ospath=faq/experimental-tpm-fallback";
const char kEolNotificationURL[] = "https://jematechnology.fr/?ospath=eol";
const char kKeyboardShortcutHelpPageUrl[] = "https://jematechnology.fr/?ospath=docs/knowledge-base/recipes/keyboard-shortcuts";
const char kJemaOSEnableWidevineLearnMoreURL[] = "https://jematechnology.fr/?ospath=docs/knowledge-base/recipes/widevine";
const char kWifiHiddenNetworkURL[] = "https://jematechnology.fr/?ospath=docs/manual/connect-your-jemaos-device/connect-to-wi-fi-&-other-networks/manage-wifi-networks/";
const char kBluetoothPairingLearnMoreUrl[] = "https://jematechnology.fr/?ospath=docs/manual/connect-your-jemaos-device/connect-to-other-devices/connect-to-bluetooth-devices/";
const char kFileManagerHelpURL[] = "https://jematechnology.fr/?ospath=docs/manual/manage-your-apps/files-and-downloads/open-save-or-delete-files/";
const char kTabletModeGesturesLearnMoreURL[] = "https://jematechnology.fr/?ospath=docs/manual/customize-settings/use-tablets/use-gestures-or-buttons-to-navigate-in-tablet-mode";
const char kRuntimeHostPermissionsHelpURL[] = "https://jematechnology.fr/?ospath=docs/manual/manage-your-apps/add-apps-and-extensions/add-apps-and-extensions/";
const char kFingerprintLearnMoreURL[] = "https://jematechnology.fr/?ospath=docs/manual/customize-settings/users-and-sync/set-up-and-sign-in-with-fingerprint-on-your-jemaos-device";

const char kJemaOSBackupRestoreLearnMoreURL[] = "https://jematechnology.fr/?ospath=docs/manual/customize-settings/jemaos-settings/misc#backup-and-restore";

const char kJemaOSToggleArcMediaAutoScanLearnMoreURL[] = "https://jematechnology.fr/?ospath=faq/disable-media-files-scan/";

const char kJemaOSDevModeTransitionLearnMoreURL[] = "https://jematechnology.fr/?ospath=docs/knowledge-base/getting-started/developer-mode";

const char kJemaOSDiscordServerURL[] = "https://discord.jematechnology.fr";
const char kJemaOSTelegramGroupURL[] = "https://telegram.jematechnology.fr";
// const char kJemaOSTelegramGroupURL[] = "https://t.me/hi_jemaos";

#if BUILDFLAG(JEMAOS_DEVICE)
const char kJemaOSProductWarrantyDefaultURL[] = "https://jematechnology.fr"; // https://warrenty.jematechnology.fr
// const char kJemaOSProductWarrantyDefaultURL[] = "https://sn.jematabduo.com";
#endif

#endif

}  // jemaos::constants
