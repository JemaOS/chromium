// Copyright 2020 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_LICENSE_MANAGER_H_
#define JEMAOS_LICENSE_MANAGER_H_
#include <memory>
#include <string>
#include "base/timer/timer.h"
#include "base/threading/thread.h"
#include "components/prefs/pref_change_registrar.h"
#include "jemaos/chromeos/ash/components/dbus/jemaos_shell_client/shell_state.h"
#include "jemaos/license/jemaos_license_fetcher.h"
#include "jemaos/license/jemaos_license_validator.h"
#include "chromeos/chromeos_export.h"
#include "chromeos/ash/components/login/login_state/login_state.h"

class Profile;

namespace jemaos::license {

class LicenseEnforcement;

enum ManagerState {
  Idle,
  Running,
};

class CHROMEOS_EXPORT LicenseManager : public ::ash::LoginState::Observer {
 public:
  class Observer {
   public:
    // Called when either the login state or the logged in user type changes.
    virtual void OnLicenseStateChanged(bool IsValid) = 0;
   protected:
    virtual ~Observer() = default;
  };
  LicenseManager();
  ~LicenseManager() override;
  static void Initialize();
  static LicenseManager* Get();
  static void Shutdown();
  void Start();
  void Stop();
  void InitCheckLicense();
  void CheckLicense();
  ManagerState GetState();
  bool IsValid();
  // Add/remove observers.
  void AddObserver(Observer* observer);
  void RemoveObserver(Observer* observer);

 private:
  void CheckLicenseInternal();
  // Reacts immediately when system A (user_session_manager) writes the
  // jemaos.subscription_active profile pref.
  void OnSubscriptionPrefChanged();
  void Reset();
  void IFetchOnlineLicense();
  void OnFetchedOnlineLicense(std::string license);
  void IFetchOfflineLicense();
  void OnFetchError(int errCode, const std::string& errMsg);
  void OnFetchedOfflineLicense(std::optional<jemaos::ash::ShellState> state);
  void IGetId();
  void IGetSerialNumber();
  void NotifyObservers();
  void OnGotId(std::optional<jemaos::ash::ShellState> state);
  void OnGotSerialNumber(std::optional<jemaos::ash::ShellState> state);
  void IValidateLicense(std::optional<std::string> license);
  void ISaveLocalLicenseString(std::optional<base::Value> license);
  void IStoreLicense(std::optional<base::Value> license);
  void OnStoredLicense(std::optional<jemaos::ash::ShellState> state);
  void OnError(int errCode, const std::string& errMsg);
  void OnValidateError(int errCode, const std::string& errMsg, std::optional<base::Value> license = std::nullopt);
  void OnValidPref(int licenseType, bool expired, int expiration_action, int showLicenseInSettings, int logOutInterval);
  void OnInvalid();
  void OnValid(std::optional<base::Value> license);
  void SkipOnline();
  void LoggedInStateChanged() override;

  void IGetOEMToken();
  void OnGotOEMToken(const std::string& token);

  bool ShouldStartEnforcement() const;
  void StartEnforcement();
  // Only business verdicts are definitive rejections (-9 expired, -7 invalid
  // signature). Everything else (transport, parse, missing context) is
  // transient and must never punish the user.
  static bool IsDefinitiveLicenseError(int errCode);

  ManagerState state_ = ManagerState::Idle;
  FetchMode mode_;
  Profile* profile_ = nullptr;
  bool is_valid_ = false;
  // One sign-out per admin seat revocation; reset when the server reactivates.
  bool seat_revocation_done_ = false;
  bool is_init_check_ = true;
  bool is_new_license_ = false;
  std::string id_ = "";
  std::string serial_number_ = "";
  std::string license_string = "";
  std::string oem_token_ = "";
  // Enforcement state lives HERE, in memory, per LicenseManager instance.
  // It used to be written to device-wide local_state prefs, so one profile's
  // expired license punished every other profile on the device.
  int enforcement_level_ = 0;
  int log_out_interval_ = 0;
  // Transient-error retry with exponential backoff (1, 2, 4, ... capped 30m).
  int transient_retry_count_ = 0;
  std::unique_ptr<base::OneShotTimer> retry_timer_;
  std::unique_ptr<base::OneShotTimer> init_timer_;
  std::unique_ptr<base::RepeatingTimer> timer_;
  std::unique_ptr<LicenseOnlineFetcher> online_fetcher_;
  std::unique_ptr<LicenseValidator> validator_;
  std::unique_ptr<LicenseEnforcement> enforcement_;
  // Watches the PROFILE pref jemaos.subscription_active, the source of truth
  // written by system A (Connect API /v1/connect/os/subscription via
  // user_session_manager). The legacy protobuf license endpoint is dead and
  // is no longer queried.
  PrefChangeRegistrar subscription_pref_registrar_;
  base::ObserverList<Observer>::Unchecked observer_list_;

  base::WeakPtrFactory<LicenseManager> weak_ptr_factory_{this};
};

}  // namespace jemaos::license


#endif  // JEMAOS_LICENSE_MANAGER_H_
