// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/prefs/jemaos_prefs.h"
#include "base/logging.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/pref_registry_simple.h"

namespace jemaos {
namespace prefs {

// Registers profile-specific preferences
void RegisterProfilePrefs(PrefRegistrySimple* registry) {
  registry->RegisterBooleanPref(kJemaOSImprovementPlanEnabled, false);
}

// Registers local state preferences
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
}

// Keeps the current preferences in sync with their corresponding values
void KeepCurrentPrefs(PrefService* local_state) {
  // Prefix with `kCurrent` prefs set here, and read only in other situations
  bool tpm_fallback = local_state->GetBoolean(jemaos::prefs::kForceTpmFallback);
  local_state->SetBoolean(kCurrentForceTpmFallback, tpm_fallback);

  bool enable_arc_ime_globally = local_state->GetBoolean(jemaos::prefs::kEnableArcIMEGlobally);
  local_state->SetBoolean(kCurrentEnableArcIMEGlobally, enable_arc_ime_globally);
}

// Marks TPM fallback as not necessary
void SetNotNecessaryForceTpmFallback(PrefService* local_state) {
  local_state->SetBoolean(kForceTpmFallbackNecessary, false);
  local_state->SetBoolean(kCurrentForceTpmFallback, false);
  local_state->SetBoolean(kForceTpmFallback, false);
}

// Clears the reboot mark preferences
void ClearRebootMarkPrefs(PrefService* local_state) {
  local_state->SetBoolean(kRebootRequiredForWidevine, false);
}

}  // namespace prefs
}  // namespace jemaos