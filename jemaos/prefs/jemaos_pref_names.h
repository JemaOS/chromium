// Copyright (c) 2021 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_PREF_NAMES_H_
#define CHROMEOS_JEMAOS_PREF_NAMES_H_

#include "jemaos/build/config/buildflags.h"

namespace jemaos {
namespace prefs {

extern const char
    kPrefHideJemaOSStoreIcon[];  // registered in
                                 // chrome/browser/ui/browser_ui_prefs.cc

extern const char kJemaOSImprovementPlanEnabled[];

// local state
extern const char kCurrentEnableArcIMEGlobally[];
extern const char kEnableArcIMEGlobally[];

extern const char kForceTpmFallbackNecessary[];
extern const char kCurrentForceTpmFallback[];
extern const char kForceTpmFallback[];

extern const char kShowSwitchTabletLaptopButton[];
extern const char kShowRebootButtonInTray[];
extern const char kShowRotateScreenButton[];

extern const char kOfflineAutoSigninAccountIdKey[];
extern const char kOfflineAutoSigninPassword[];
extern const char kOfflineAutoSigninPasswordFormat[];
extern const char kOfflineAutoSigninIsChromeLastSignout[];

extern const char kConnectApiUserId[];
extern const char kConnectApiUserStatus[];

// Whether the Jema online account of this profile has an active subscription
// (Pro/Pro+). Written after a successful /v1/connect/os/subscription check;
// false for local accounts, Freemium accounts, or when the check never ran.
extern const char kJemaSubscriptionActive[];

// same with kFactoryResetRequested in chrome/common/pref_names.cc
// dep conflict issue
extern const char kFactoryResetRequested[];

extern const char kRebootRequiredForWidevine[];

extern const char kJemaAssistantEnabled[];
extern const char kJemaAssistantExtraAcceleratorEnabled[];

// JemaOS auth token lifecycle (OSCrypt-encrypted, persists across restarts).
extern const char kJemaOsAuthAccessTokenEncrypted[];
extern const char kJemaOsAuthRefreshTokenEncrypted[];
extern const char kJemaOsAuthEmail[];
extern const char kJemaOsAuthIssuedAt[];

extern const char kJemaOSArcMediaAutoScanEnabled[];

#if BUILDFLAG(USE_JEMAOS_LICENSE)
extern const char kJemaLicenseShouldShowInSettings[];
extern const char kJemaLicenseStateType[];
extern const char kJemaLicenseEnforcementLevel[];
extern const char kJemaLicenseEnforcementLogOutInterval[];
#endif

#if BUILDFLAG(USE_JEMAOS_COM)
extern const char kCrostiniInstallerNotificationUserInteracted[];
#endif

}  // namespace prefs
}  // namespace jemaos

#endif
