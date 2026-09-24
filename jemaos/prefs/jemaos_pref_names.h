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
extern const char kJemaOsDeviceUid[];
extern const char kConnectApiUserStatus[];

// Whether the Jema online account of this profile has an active subscription
// (Pro/Pro+). Written after a successful /v1/connect/os/subscription check;
// false for local accounts, Freemium accounts, or when the check never ran.
extern const char kJemaSubscriptionActive[];

// same with kFactoryResetRequested in chrome/common/pref_names.cc
// dep conflict issue
extern const char kFactoryResetRequested[];

extern const char kRebootRequiredForWidevine[];

// Whether CPU Turbo Boost is enabled (JemaOS Settings toggle, applied
// via /usr/share/jemaos_shell/cpu_turbo.sh). Default true.
extern const char kCpuTurboEnabled[];

extern const char kJemaAssistantEnabled[];
extern const char kJemaAssistantExtraAcceleratorEnabled[];

// JemaOS auth token lifecycle (OSCrypt-encrypted, persists across restarts).
extern const char kJemaOsAuthAccessTokenEncrypted[];
extern const char kJemaOsAuthRefreshTokenEncrypted[];
extern const char kJemaOsAuthEmail[];
extern const char kJemaOsAuthIssuedAt[];

// Monotonic SaaS password version last seen by this device. When the value
// returned by osLogin/getOsToken/refreshtoken differs, the SaaS password was
// changed and the device must re-seal its cryptohome key (force an online
// sign-in so the standard password-change flow runs).
extern const char kJemaOsAuthPasswordVersion[];

// Per-account map (email -> OSCrypt ciphertext of the last password that
// successfully unlocked the cryptohome). JemaOS uses it to re-seal the
// cryptohome key automatically after a SaaS password change, so the user is
// never asked for their old password.
extern const char kJemaOsSavedPasswords[];

// Per-account map (email -> OSCrypt ciphertext of a random per-account vault
// secret). For Jema ONLINE accounts the cryptohome vault is sealed with this
// secret instead of the SaaS password, so a SaaS password change never requires
// re-sealing the vault: the pod validates the typed password online and unlocks
// with this stable secret.
extern const char kJemaOsVaultSecrets[];

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
