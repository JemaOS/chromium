// Copyright (c) 2021 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/prefs/jemaos_prefs.h"
#include "base/logging.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_registry_simple.h"
#include "jemaos/constants/jemaos_constants.h"

namespace jemaos {
namespace prefs {

void RegisterProfilePrefs(PrefRegistrySimple* registry) {
  registry->RegisterBooleanPref(kJemaOSImprovementPlanEnabled, false);

  registry->RegisterBooleanPref(kJemaAssistantEnabled, true);
  registry->RegisterBooleanPref(kJemaAssistantExtraAcceleratorEnabled, true);

  registry->RegisterBooleanPref(kJemaOSArcMediaAutoScanEnabled, true);
#if BUILDFLAG(USE_JEMAOS_COM)
  registry->RegisterBooleanPref(kCrostiniInstallerNotificationUserInteracted, false);
#endif
}

void RegisterLocalStatePrefs(PrefRegistrySimple* registry) {
  registry->RegisterBooleanPref(kForceTpmFallbackNecessary, true);
  registry->RegisterBooleanPref(kCurrentForceTpmFallback, false);
  registry->RegisterBooleanPref(kForceTpmFallback, false);

  registry->RegisterBooleanPref(kEnableArcIMEGlobally, false);
  registry->RegisterBooleanPref(kCurrentEnableArcIMEGlobally, false);

  registry->RegisterStringPref(kOfflineAutoSigninAccountIdKey, std::string());
  registry->RegisterStringPref(kOfflineAutoSigninPassword, std::string());
  registry->RegisterStringPref(kOfflineAutoSigninPasswordFormat, std::string());
  registry->RegisterBooleanPref(kOfflineAutoSigninIsChromeLastSignout, false);

  registry->RegisterBooleanPref(kRebootRequiredForWidevine, false);

#if BUILDFLAG(USE_JEMAOS_LICENSE)
  registry->RegisterBooleanPref(kJemaLicenseShouldShowInSettings, false);
  registry->RegisterIntegerPref(kJemaLicenseStateType,
                                static_cast<int>(jemaos::constants::LicenseStateType::kUnspecified));
  registry->RegisterIntegerPref(kJemaLicenseEnforcementLevel,
                                static_cast<int>(jemaos::constants::LicenseEnforcementLevel::kNone));
  registry->RegisterIntegerPref(kJemaLicenseEnforcementLogOutInterval, 0);
#endif
}

void KeepCurrentPrefs(PrefService* local_state) {
  // prefix with `kCurrent` prefs set here, and read only in other situation
  bool tpm_fallback = local_state->GetBoolean(jemaos::prefs::kForceTpmFallback);
  local_state->SetBoolean(kCurrentForceTpmFallback, tpm_fallback);

  bool enable_arc_ime_globally = local_state->GetBoolean(jemaos::prefs::kEnableArcIMEGlobally);
  local_state->SetBoolean(kCurrentEnableArcIMEGlobally, enable_arc_ime_globally);
}

void SetNotNecessaryForceTpmFallback(PrefService* local_state) {
  local_state->SetBoolean(kForceTpmFallbackNecessary, false);
  local_state->SetBoolean(kCurrentForceTpmFallback, false);
  local_state->SetBoolean(kForceTpmFallback, false);
}

void ClearRebootMarkPrefs(PrefService* local_state) {
  local_state->SetBoolean(kRebootRequiredForWidevine, false);
}

void ClearOneShotProfilePrefs(PrefService* prefs) {
  prefs->ClearPref(kJemaOSArcMediaAutoScanEnabled);
}

} // prefs
} // jemaos
