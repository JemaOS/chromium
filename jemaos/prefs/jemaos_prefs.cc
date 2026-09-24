// Copyright (c) 2021 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/prefs/jemaos_prefs.h"

#include "base/base64.h"
#include "base/logging.h"
#include "base/strings/string_util.h"
#include "base/values.h"
#include "components/os_crypt/sync/os_crypt.h"
#include "components/prefs/pref_registry_simple.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"
#include "jemaos/constants/jemaos_constants.h"

namespace jemaos {
namespace prefs {

void RegisterProfilePrefs(PrefRegistrySimple* registry) {
  registry->RegisterBooleanPref(kJemaOSImprovementPlanEnabled, false);

  registry->RegisterBooleanPref(kJemaAssistantEnabled, false);
  registry->RegisterBooleanPref(kJemaAssistantExtraAcceleratorEnabled, false);

  registry->RegisterBooleanPref(kJemaOSArcMediaAutoScanEnabled, true);

  // Set to true once the Jema online account of this profile proves an
  // active Pro/Pro+ subscription via the Connect API. Stays false for local
  // accounts and Freemium accounts.
  registry->RegisterBooleanPref(kJemaSubscriptionActive, false);
#if BUILDFLAG(USE_JEMAOS_COM)
  registry->RegisterBooleanPref(kCrostiniInstallerNotificationUserInteracted,
                                false);
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

  registry->RegisterStringPref(kConnectApiUserId, std::string());
  registry->RegisterStringPref(kJemaOsDeviceUid, std::string());
  registry->RegisterStringPref(kConnectApiUserStatus, std::string());

  registry->RegisterStringPref(kJemaOsAuthAccessTokenEncrypted, std::string());
  registry->RegisterStringPref(kJemaOsAuthRefreshTokenEncrypted, std::string());
  registry->RegisterStringPref(kJemaOsAuthEmail, std::string());
  registry->RegisterInt64Pref(kJemaOsAuthIssuedAt, 0);
  registry->RegisterInt64Pref(kJemaOsAuthPasswordVersion, 0);
  registry->RegisterDictionaryPref(kJemaOsSavedPasswords);
  registry->RegisterDictionaryPref(kJemaOsVaultSecrets);

  registry->RegisterBooleanPref(kRebootRequiredForWidevine, false);
  registry->RegisterBooleanPref(kCpuTurboEnabled, true);

#if BUILDFLAG(USE_JEMAOS_LICENSE)
  registry->RegisterBooleanPref(kJemaLicenseShouldShowInSettings, false);
  registry->RegisterIntegerPref(
      kJemaLicenseStateType,
      static_cast<int>(jemaos::constants::LicenseStateType::kUnspecified));
  registry->RegisterIntegerPref(
      kJemaLicenseEnforcementLevel,
      static_cast<int>(jemaos::constants::LicenseEnforcementLevel::kNone));
  registry->RegisterIntegerPref(kJemaLicenseEnforcementLogOutInterval, 0);
#endif
}

void KeepCurrentPrefs(PrefService* local_state) {
  // prefix with `kCurrent` prefs set here, and read only in other situation
  bool tpm_fallback = local_state->GetBoolean(jemaos::prefs::kForceTpmFallback);
  local_state->SetBoolean(kCurrentForceTpmFallback, tpm_fallback);

  bool enable_arc_ime_globally =
      local_state->GetBoolean(jemaos::prefs::kEnableArcIMEGlobally);
  local_state->SetBoolean(kCurrentEnableArcIMEGlobally,
                          enable_arc_ime_globally);
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

void SaveJemaAccountPassword(PrefService* local_state,
                             const std::string& email,
                             const std::string& password) {
  if (!local_state || email.empty() || password.empty()) {
    return;
  }
  std::string encrypted;
  if (!OSCrypt::EncryptString(password, &encrypted)) {
    LOG(ERROR) << "[JEMAOS] Failed to encrypt saved password";
    return;
  }
  const std::string key = base::ToLowerASCII(email);
  ScopedDictPrefUpdate update(local_state, kJemaOsSavedPasswords);
  update->Set(key, base::Base64Encode(encrypted));
  VLOG(1) << "[JEMAOS] Saved device password for " << key;
}

std::string GetJemaAccountPassword(PrefService* local_state,
                                   const std::string& email) {
  if (!local_state || email.empty()) {
    return std::string();
  }
  const base::Value::Dict& dict = local_state->GetDict(kJemaOsSavedPasswords);
  const std::string* encoded = dict.FindString(base::ToLowerASCII(email));
  if (!encoded || encoded->empty()) {
    return std::string();
  }
  std::string encrypted;
  std::string plain;
  if (!base::Base64Decode(*encoded, &encrypted) ||
      !OSCrypt::DecryptString(encrypted, &plain)) {
    LOG(WARNING) << "[JEMAOS] Failed to decrypt saved password";
    return std::string();
  }
  return plain;
}

void SaveJemaVaultSecret(PrefService* local_state,
                         const std::string& email,
                         const std::string& secret) {
  if (!local_state || email.empty() || secret.empty()) {
    return;
  }
  std::string encrypted;
  if (!OSCrypt::EncryptString(secret, &encrypted)) {
    LOG(ERROR) << "[JEMAOS] Failed to encrypt vault secret";
    return;
  }
  ScopedDictPrefUpdate update(local_state, kJemaOsVaultSecrets);
  update->Set(base::ToLowerASCII(email), base::Base64Encode(encrypted));
}

std::string GetJemaVaultSecret(PrefService* local_state,
                               const std::string& email) {
  if (!local_state || email.empty()) {
    return std::string();
  }
  const base::Value::Dict& dict = local_state->GetDict(kJemaOsVaultSecrets);
  const std::string* encoded = dict.FindString(base::ToLowerASCII(email));
  if (!encoded || encoded->empty()) {
    return std::string();
  }
  std::string encrypted;
  std::string plain;
  if (!base::Base64Decode(*encoded, &encrypted) ||
      !OSCrypt::DecryptString(encrypted, &plain)) {
    LOG(WARNING) << "[JEMAOS] Failed to decrypt vault secret";
    return std::string();
  }
  return plain;
}

}  // namespace prefs
}  // namespace jemaos
