// Copyright 2020 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/license/jemaos_license_manager.h"
#include <algorithm>
#include "base/values.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/files/file_util.h"
#include "base/task/thread_pool.h"
#include "chrome/browser/ash/login/oobe_configuration.h"
#include "chromeos/ash/components/network/portal_detector/network_portal_detector.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ash/policy/enrollment/enrollment_token_provider.h"
#include "components/prefs/pref_service.h"
#include "chromeos/ash/components/network/network_state_handler.h"
#include "jemaos/chromeos/ash/components/dbus/jemaos_shell_client/jemaos_shell_client.h"
#include "jemaos/chromeos/ash/components/dbus/jemaos_shell_client/shell_state.h"
#include "jemaos/license/jemaos_callback_status.h"
#include "jemaos/switches/license/license_constants.h"
#include "jemaos/license/jemaos_license_enforcement.h"
#include "chrome/browser/lifetime/application_lifetime.h"

#include "jemaos/switches/license/license_switches.h"
#include "base/strings/string_number_conversions.h"
#include "jemaos/constants/jemaos_constants.h"
#include "jemaos/prefs/jemaos_pref_names.h"

using JemaOSShellClient = jemaos::ash::JemaOSShellClient;
using ShellState = jemaos::ash::ShellState;
using LicenseStateType = jemaos::constants::LicenseStateType;
using LicenseEnforcementLevel = jemaos::constants::LicenseEnforcementLevel;

namespace jemaos::license {
namespace  {
  const char kShellCmd[] = "/usr/share/jemaos_shell/license-utils.sh %s";
  const char kParamId[] = "id";
  const char kParamReadLicense[] = "read";
  const char kParamWriteLicense[] = "write";
  const char *kLicenseAttrs[] = {"expire_date", "license_type", "license", "expiration_action", "show_license_in_settings", "log_out_interval"};
  const char kVpdCmd[] = "vpd -g %s";
  const char kParamSerialNumber[] = "serial_number";
  const int kTestLicenseCheckRepeatTimeInMinutes = 1;
  const int kLicenseCheckRepeatTimeInMinutes = 30;
  const int kLicenseCheckDelayTimeInSeconds = 30;
  LicenseManager* g_license_manager = nullptr;

  JemaOSShellClient* GetShellClient() {
    return JemaOSShellClient::Get();
  }

  std::string GetIDCmd() {
    return base::StringPrintf(kShellCmd, kParamId);
  }

  std::string GetSerialNumberCmd() {
    return base::StringPrintf(kVpdCmd, kParamSerialNumber);
  }

  std::string GetLicenseCmd() {
    return base::StringPrintf(kShellCmd, kParamReadLicense);
  }

  std::string SaveLicenseCmd(base::Value::Dict* license) {
    std::string result(kParamWriteLicense);
    const std::string* value;
    std::optional<int> int_value;
    for (auto & kLicenseAttr : kLicenseAttrs) {
      if ((value = license->FindString(kLicenseAttr))) {
        result.append(" ").append(kLicenseAttr).append("=").append(*value);
      } else if ((int_value = license->FindInt(kLicenseAttr))) {
        result.append(" ").append(kLicenseAttr).append("=").append(base::NumberToString(int_value.value()));
      }
    }
    return base::StringPrintf(kShellCmd, result.c_str());
  }
}  // namespace

using jemaos::constants::LicenseEnforcementLevel;

void LicenseManager::Initialize() {
  CHECK(g_license_manager == nullptr);
  g_license_manager = new LicenseManager();
}


LicenseManager* LicenseManager::Get() {
  return g_license_manager;
}

void LicenseManager::Shutdown() {
  CHECK(g_license_manager != nullptr);
  g_license_manager->Stop();
  delete g_license_manager;
  g_license_manager = nullptr;
}

LicenseManager::LicenseManager():
    mode_(FetchMode::OfflineMode),
    retry_timer_(std::make_unique<base::OneShotTimer>()),
    init_timer_(std::make_unique<base::OneShotTimer>()),
    timer_(std::make_unique<base::RepeatingTimer>()),
    online_fetcher_(std::make_unique<LicenseOnlineFetcher>()),
    validator_(std::make_unique<LicenseValidator>()),
    enforcement_(std::make_unique<LicenseEnforcement>()) {
  if (::ash::LoginState::IsInitialized())
    ::ash::LoginState::Get()->AddObserver(this);
  VLOG(1) << "LicenseManager inited.";
}

LicenseManager::~LicenseManager() = default;

void LicenseManager::Start() {
  IGetId();
  IGetSerialNumber();
  if (!init_timer_->IsRunning()) {
    init_timer_->Start(
    FROM_HERE,
    base::Seconds(kLicenseCheckDelayTimeInSeconds),
    base::BindOnce(&LicenseManager::InitCheckLicense, base::Unretained(this)));
  }
  if (!timer_->IsRunning()) {
    VLOG(1) << "Start timer of checking.";
    timer_->Start(
    FROM_HERE,
    base::Minutes(jemaos::switches::IsLicenseTestMode() ?
      kTestLicenseCheckRepeatTimeInMinutes : kLicenseCheckRepeatTimeInMinutes),
    base::BindRepeating(&LicenseManager::CheckLicense, weak_ptr_factory_.GetWeakPtr()));
  }
}

void LicenseManager::Stop() {
  if (init_timer_ && init_timer_->IsRunning())
    init_timer_->Stop();
  if (timer_ && timer_->IsRunning())
    timer_->Stop();
  if (retry_timer_ && retry_timer_->IsRunning())
    retry_timer_->Stop();
  subscription_pref_registrar_.RemoveAll();
  Reset();
}

void LicenseManager::InitCheckLicense() {
  is_init_check_ = true;
  CheckLicenseInternal();
}

 void LicenseManager::CheckLicense() {
  is_init_check_ = false;
  CheckLicenseInternal();
}


void LicenseManager::CheckLicenseInternal() {
  //---***JEMAOS BEGIN***---
  // Source of truth = system A: the Connect API /v1/connect/os/subscription,
  // whose verdict user_session_manager writes into the PROFILE pref
  // jemaos.subscription_active. The legacy protobuf endpoint
  // (apis.jematechnology.fr/license/chromeos/{id}) is dead (HTTP 200 with an
  // empty body for every id) and is NO LONGER QUERIED: its empty answers
  // were classified as definitive rejections (-1) and caused the sign-out
  // loop. No fetch, no punishment, in either branch below.
  // This runs early at startup: browser process / profile manager may not
  // exist yet. Never dereference them unguarded.
  if (!g_browser_process) {
    state_ = ManagerState::Idle;
    return;
  }
  ProfileManager* profile_manager = g_browser_process->profile_manager();
  if (!profile_manager) {
    state_ = ManagerState::Idle;
    return;
  }
  Profile* active_profile = profile_manager->GetActiveUserProfile();
  if (active_profile && active_profile != profile_) {
    profile_ = active_profile;
    // Reset(), not RemoveAll(): RemoveAll() keeps the old PrefService
    // pointer, and Init() DCHECKs IsEmpty() || service_ == service, which
    // would fail when switching to another profile's PrefService.
    subscription_pref_registrar_.Reset();
    if (profile_->GetPrefs()) {
      subscription_pref_registrar_.Init(profile_->GetPrefs());
      subscription_pref_registrar_.Add(
          jemaos::prefs::kJemaSubscriptionActive,
          base::BindRepeating(&LicenseManager::OnSubscriptionPrefChanged,
                              weak_ptr_factory_.GetWeakPtr()));
      subscription_pref_registrar_.Add(
          jemaos::prefs::kJemaSeatDeactivated,
          base::BindRepeating(&LicenseManager::OnSubscriptionPrefChanged,
                              weak_ptr_factory_.GetWeakPtr()));
    }
  }
  const bool subscription_active =
      profile_ && profile_->GetPrefs() &&
      profile_->GetPrefs()->GetBoolean(jemaos::prefs::kJemaSubscriptionActive);
  const bool seat_deactivated =
      profile_ && profile_->GetPrefs() &&
      profile_->GetPrefs()->GetBoolean(jemaos::prefs::kJemaSeatDeactivated);

  // Explicit admin revocation (all seats deactivated): close the session.
  // This is NOT the freemium path - the admin deliberately cut access. One
  // clean sign-out per revocation (guarded), never a repeating loop: after
  // sign-out LoginState fires Stop() and the flag stays set until the server
  // says otherwise, so a re-login attempt is re-checked and refused again by
  // the login gate, without crash-looping the browser.
  if (seat_deactivated) {
    state_ = ManagerState::Idle;
    if (!seat_revocation_done_) {
      seat_revocation_done_ = true;
      LOG(WARNING) << "[JEMAOS] Seats deactivated by fleet admin: signing out.";
      if (enforcement_) {
        enforcement_->StopEnforcement();
      }
      chrome::AttemptUserExit();
    }
    return;
  }
  seat_revocation_done_ = false;

  state_ = ManagerState::Idle;
  transient_retry_count_ = 0;
  if (retry_timer_ && retry_timer_->IsRunning()) {
    retry_timer_->Stop();
  }

  if (subscription_active) {
    // Active subscription: license valid. No enforcement, no protobuf fetch.
    if (!is_valid_) {
      is_valid_ = true;
      NotifyObservers();
    }
    // Also removes any freemium notice left from before the subscription
    // check answered.
    if (enforcement_) {
      enforcement_->StopEnforcement();
    }
    if (init_timer_) {
      init_timer_->Stop();
    }
    VLOG(1) << "[JEMAOS] Subscription active (system A): license valid.";
    return;
  }

  // Freemium / local account / subscription not proven: NEVER punish.
  // No logout, no forced quit, no repeated notifications - at most one
  // discreet notice per session, removed if the subscription activates.
  if (is_valid_) {
    is_valid_ = false;
    NotifyObservers();
  }
  VLOG(1) << "[JEMAOS] No active subscription (system A): freemium mode, "
             "no enforcement.";
  if (profile_ && enforcement_) {
    enforcement_->ShowFreemiumNotice(profile_);
  }
  //---***JEMAOS END***---
}

void LicenseManager::OnSubscriptionPrefChanged() {
  // System A just wrote a new verdict; re-evaluate immediately instead of
  // waiting for the next periodic check.
  CheckLicenseInternal();
}

void LicenseManager::IGetId() {
  GetShellClient()->SyncExec(GetIDCmd(),
    base::BindOnce(&LicenseManager::OnGotId, weak_ptr_factory_.GetWeakPtr()));
}

void LicenseManager::OnGotId(std::optional<ShellState> state) {
  if (!state || state->code != 0) {
    LOG(ERROR) << "Can't get license id.\n";
  } else {
    VLOG(1) << "Get id:" << state->result;
    id_ = state->result;
    id_.erase(std::remove(id_.begin(), id_.end(), '\n'), id_.end());
  }
}

void LicenseManager::IGetOEMToken() {
  auto* oobe_configuration = ::ash::OobeConfiguration::Get();
  auto token = policy::GetJemaEnrollmentToken(oobe_configuration);
  if (token.has_value()) {
    OnGotOEMToken(token.value());
  }
}

void LicenseManager::OnGotOEMToken(const std::string& token) {
  if (!token.empty()) {
    VLOG(1) << "Get OEM token:" << token;
    oem_token_ = token;
  }
}

void LicenseManager::IGetSerialNumber() {
  GetShellClient()->SyncExec(GetSerialNumberCmd(),
    base::BindOnce(&LicenseManager::OnGotSerialNumber, weak_ptr_factory_.GetWeakPtr()));
}

void LicenseManager::OnGotSerialNumber(std::optional<ShellState> state) {
  if (!state || state->code != 0) {
    LOG(ERROR) << "Can't get serial_number.\n";
  } else {
    VLOG(1) << "Get serial_number:" << state->result;
    serial_number_ = state->result;
    serial_number_.erase(
        std::remove(serial_number_.begin(), serial_number_.end(), '\n'),
        serial_number_.end());
  }
  IGetOEMToken();
}

void LicenseManager::OnFetchError(int errCode, const std::string& errMsg) {
  if (mode_ == FetchMode::OfflineMode) {
    return;
  }
  LOG(ERROR) << "Fetch online license verification error";
  // Transport failures (timeout -30, no network context -40, HTTP errors)
  // must reset the state machine and go through the transient-retry path,
  // not leave state_ stuck at Running (which froze all later checks).
  OnError(errCode, errMsg);
}

void LicenseManager::OnValidateError(int errCode, const std::string& errMsg, std::optional<base::Value> license) {
  // NEVER persist an expired license. It used to be stored here and replayed
  // from the cache at every boot/login, re-arming enforcement each time:
  // that was the sign-out loop. Only licenses that passed validation are
  // stored (OnValid -> IStoreLicense).
  OnError(errCode, errMsg);
}

bool LicenseManager::IsDefinitiveLicenseError(int errCode) {
  // Business verdicts only: -9 = license expired, -7 = invalid signature.
  // Everything else (-1 empty answer, -2/-3/-4/-5 parse/missing fields,
  // -6/-8/-10/-11/-12/-13 malformed payload, -30 timeout, -40 no network
  // context, HTTP status codes) is transient or a server/client bug and
  // must NEVER punish the user.
  return errCode == -9 || errCode == -7;
}

void LicenseManager::OnError(int errCode, const std::string& errMsg) {
  if (mode_ == FetchMode::OfflineMode) {
    LOG(WARNING) << "Error occured from offline verification, code:"
      << errCode << " message:" << errMsg << " Switch to online verification.";
    is_new_license_ = (errCode == -3); // no license field
    mode_ = FetchMode::OnlineMode;
    IFetchOnlineLicense();
  } else {
    LOG(ERROR) << "Error occured from online verification, code:"
      << errCode << " message:" << errMsg;
    //---***JEMAOS BEGIN***---
    // Only a DEFINITIVE business rejection (-9 expired, -7 bad signature)
    // may enforce. The old test (errCode < 0 && errCode != -30) also caught
    // transport/context errors (-1, -40, ...) and punished users for being
    // offline or for a server hiccup.
    if (!IsDefinitiveLicenseError(errCode)) {
      LOG(WARNING) << "[JEMAOS] License verification unavailable (code "
                   << errCode << "); transient, no enforcement, retry with backoff";
      state_ = ManagerState::Idle;
      mode_ = FetchMode::OnlineMode;
      // Exponential backoff: 1, 2, 4, 8, 16, then capped at 30 minutes.
      if (retry_timer_ && !retry_timer_->IsRunning()) {
        const int delay_minutes =
            std::min(1 << std::min(transient_retry_count_, 5), 30);
        ++transient_retry_count_;
        retry_timer_->Start(
            FROM_HERE, base::Minutes(delay_minutes),
            base::BindOnce(&LicenseManager::CheckLicenseInternal,
                           weak_ptr_factory_.GetWeakPtr()));
      }
      return;
    }
    //---***JEMAOS END***---
    transient_retry_count_ = 0;
    OnInvalid();
  }
}

void LicenseManager::OnValid(std::optional<base::Value> license) {
  VLOG(1) << "license is valid.";
  transient_retry_count_ = 0;
  if (retry_timer_ && retry_timer_->IsRunning()) {
    retry_timer_->Stop();
  }
  if (mode_ == FetchMode::OfflineMode && is_init_check_) {
    ISaveLocalLicenseString(std::move(license));
    mode_ = FetchMode::OnlineMode;
    IFetchOnlineLicense();
    return;
  }
  is_valid_ = true;
  state_ = ManagerState::Idle;
  if (init_timer_) {
    init_timer_->Stop();
  }
  if (timer_) {
    timer_->Stop();
  }
  if (mode_ == FetchMode::OfflineMode) {
    // save string, to prevent call shellclient to store the same license get from online
    ISaveLocalLicenseString(std::move(license));
    // always fetch online
    mode_ = FetchMode::OnlineMode;
    IFetchOnlineLicense();
  } else if (mode_ == FetchMode::OnlineMode) {
    IStoreLicense(std::move(license));
  }
  Reset();
}

void LicenseManager::Reset() {
  // restore punishment
  VLOG(1) << "Reset invalid effect.";
  NotifyObservers();
  if (enforcement_) {
    enforcement_->StopEnforcement();
  }
}

bool LicenseManager::ShouldStartEnforcement() const {
  return !is_init_check_;
}

void LicenseManager::OnInvalid() {
  // Legacy protobuf path: a rejection here must NEVER punish the user.
  // The source of truth is system A (kJemaSubscriptionActive); this
  // callback only resets the legacy state machine and logs. It no longer
  // touches is_valid_ nor starts enforcement.
  state_ = ManagerState::Idle;
  mode_ = FetchMode::OfflineMode;
  LOG(WARNING) << "[JEMAOS] Legacy license validation rejected; ignored "
                  "(the subscription pref is the source of truth).";
}

void LicenseManager::StartEnforcement() {
  // Disabled: punishment is no longer driven by the legacy protobuf
  // license. System A (kJemaSubscriptionActive) gates premium features;
  // freemium users are never logged out. Signature kept for compatibility.
  LOG(WARNING) << "[JEMAOS] StartEnforcement called but enforcement is "
                  "disabled (subscription-based licensing).";
}

void LicenseManager::SkipOnline() {
  state_ = ManagerState::Idle;
  mode_ = FetchMode::OfflineMode;
}

void LicenseManager::IValidateLicense(std::optional<std::string> license) {
  if (license.has_value())
    VLOG(1) << "Start validating." << license.value();
  validator_->StartValidate(id_,
                            std::move(license),
                            base::BindOnce(
                              &LicenseManager::OnValid,
                              weak_ptr_factory_.GetWeakPtr()),
    base::BindOnce(&LicenseManager::OnValidPref, weak_ptr_factory_.GetWeakPtr()),
    base::BindOnce(&LicenseManager::OnValidateError, weak_ptr_factory_.GetWeakPtr()));
}

void LicenseManager::OnValidPref(int licenseType, bool expired, int expirationAction, int showLicenseInSettings, int logOutInterval) {
  if (!g_browser_process) return;
  if (!profile_) {
    ProfileManager* profile_manager = g_browser_process->profile_manager();
    if (!profile_manager) return;
    profile_ = profile_manager->GetActiveUserProfile();
  }
  if (!profile_ || !profile_->GetPrefs()) return;

  PrefService* prefs = g_browser_process->local_state();
  if (!prefs) return;

  LicenseStateType state = LicenseStateType::kUnspecified;
  switch (licenseType) {
    // trial
    case 0:
      state = expired ? LicenseStateType::kLicenseForYouExpired : LicenseStateType::kLicenseForYouTrial;
      break;
    // valid
    case 1:
      state = expired ? LicenseStateType::kLicenseForYouExpired : LicenseStateType::kLicenseForYouValid;
      break;
    // enterprise trial
    case 2:
      state = expired ? LicenseStateType::kLicenseForEnterpriseExpired : LicenseStateType::kLicenseForEnterpriseTrial;
      break;
    // enterprise valid
    case 3:
      state = expired ? LicenseStateType::kLicenseForEnterpriseExpired : LicenseStateType::kLicenseForEnterpriseValid;
      break;
    // unlicensed
    case 4:
      state = LicenseStateType::kUnlicensed;
      break;
    default:
      state = LicenseStateType::kUnspecified;
      break;
  }
  prefs->SetInteger(jemaos::prefs::kJemaLicenseStateType, static_cast<int>(state));

  VLOG(2) << "license pref, state: " << static_cast<int>(state)
    << "expiration action: " << expirationAction
    << " show license in settings: " << showLicenseInSettings
    << " log out interval: " << logOutInterval;

  LicenseEnforcementLevel level = LicenseEnforcementLevel::kNone;
  switch (expirationAction) {
    case 0:
      level = LicenseEnforcementLevel::kNone;
      break;
    case 1:
      level = LicenseEnforcementLevel::kForcePopup;
      break;
    case 2:
      level = LicenseEnforcementLevel::kForceQuit;
      break;
    default:
      level = LicenseEnforcementLevel::kNone;
      break;
  }
  // Punishment state stays IN MEMORY (per LicenseManager), never in
  // device-wide local_state: one profile's expired license must not punish
  // the other profiles of the device.
  enforcement_level_ = static_cast<int>(level);
  log_out_interval_ = logOutInterval;

  prefs->SetBoolean(jemaos::prefs::kJemaLicenseShouldShowInSettings, showLicenseInSettings == 1);
}

void LicenseManager::ISaveLocalLicenseString(std::optional<base::Value> license) {
  base::Value::Dict *licenseDict = license->GetIfDict();
  if (!licenseDict) return;
  const std::string* value;
  if ((value = licenseDict->FindString("license"))) {
    license_string = *value;
  } else {
    license_string.clear();
  }
}

void LicenseManager::IStoreLicense(std::optional<base::Value> license) {
  VLOG(1) << "Store license";
  base::Value::Dict *licenseDict = license->GetIfDict();
  const std::string* value;
  if (licenseDict) {
    if ((value = licenseDict->FindString("license")) && *value == license_string) {
      VLOG(1) << "License is not changed, skip store.";
      return;
    }
    GetShellClient()->SyncExec(SaveLicenseCmd(licenseDict),
      base::BindOnce(&LicenseManager::OnStoredLicense,
        weak_ptr_factory_.GetWeakPtr()));
  }
}

void LicenseManager::OnStoredLicense(std::optional<ShellState> state) {
  if ( state && (state->code != 0))
    LOG(WARNING) << "Store license got error:" << state->result;
  else
    VLOG(1) << "Store license successfully";
}

void LicenseManager::IFetchOfflineLicense() {
  GetShellClient()->SyncExec(GetLicenseCmd(),
    base::BindOnce(
      &LicenseManager::OnFetchedOfflineLicense, weak_ptr_factory_.GetWeakPtr()));
}

void LicenseManager::OnFetchedOfflineLicense(std::optional<ShellState> state) {
  if (!state) {
    OnError(-1, "No shell result.");
  } else {
    VLOG(1) << "Get offline license:" << state->result;
    if (state->code == 0)
      IValidateLicense(state->result);
    else
      OnError(state->code, state->result);
  }
}

void LicenseManager::LoggedInStateChanged() {
  if (::ash::LoginState::Get()->IsUserLoggedIn())
    Start();
  else
    Stop();
}

void LicenseManager::IFetchOnlineLicense() {
  ::ash::NetworkStateHandler* network_state_handler =
      ::ash::NetworkHandler::Get()->network_state_handler();
  if (!network_state_handler) {
    // Reset the state machine, otherwise state_ stays Running and every
    // later check (including backoff retries) is skipped forever.
    SkipOnline();
    return;
  }
  const ::ash::NetworkState* network = network_state_handler->DefaultNetwork();
  if (!network) {
    SkipOnline();
    return;
  }
  ::ash::NetworkState::PortalState portalStatus = network->portal_state();
  if (portalStatus !=
      ::ash::NetworkState::PortalState::kOnline) {
    VLOG(1) << "Not online currently, skip fetching online license";
    SkipOnline();
    return;
  }
  online_fetcher_->StartFetch(id_,
                              serial_number_,
                              is_new_license_,
                              oem_token_,
                              base::BindOnce(
                                &LicenseManager::IValidateLicense,
                                weak_ptr_factory_.GetWeakPtr()),
    base::BindOnce(&LicenseManager::OnFetchError, weak_ptr_factory_.GetWeakPtr()));
}

void LicenseManager::AddObserver(Observer* observer) {
  observer_list_.AddObserver(observer);
}

void LicenseManager::RemoveObserver(Observer* observer) {
  observer_list_.RemoveObserver(observer);
}

void LicenseManager::NotifyObservers() {
  for (auto& observer : observer_list_)
    observer.OnLicenseStateChanged(is_valid_);
}

}  // namespace jemaos::license
