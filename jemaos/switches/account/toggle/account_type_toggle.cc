// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/account/toggle/account_type_toggle.h"
#include "jemaos/switches/account/account_switches.h"
#include "jemaos/switches/account/base/account_base.h"

#include "google_apis/gaia/gaia_urls.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/policy/chrome_browser_policy_connector.h"
#include "chrome/browser/browser_process_platform_part_chromeos.h"
#include "components/user_manager/user_manager.h"
#include "components/account_id/account_id.h"
#include "base/command_line.h"
#include "remoting/base/service_urls.h"

namespace jemaos {
namespace switches {

namespace {

// Resets URLs for Gaia and remoting services
void ResetUrls() {
  GaiaUrls* gaia_urls = GaiaUrls::GetInstance();
  if (gaia_urls) {
    gaia_urls->Reset();
  }

  remoting::ServiceUrls* service_urls = remoting::ServiceUrls::GetInstance();
  if (service_urls) {
    service_urls->ResetServerEndpoints();
  }

  if (!g_browser_process || !g_browser_process->platform_part()) return;
  policy::ChromeBrowserPolicyConnector* connector =
      g_browser_process->browser_policy_connector();
  if (connector) {
    connector->ResetDeviceManagementServiceConfiguration();
  }
}

// Toggles the Jema account flag internally based on the account type
void ToggleJemaAccountFlagInternal(base::CommandLine* cmdline, const AccountType account_type) {
  if (cmdline->HasSwitch(kJemaAccountForceDisabledForTest)) return;
  if (IsJemaSetDeviceManaged()) return;
  bool need_reset_url = false;
  if (account_type == AccountType::JEMA_ACCOUNT) {
    if (!cmdline->HasSwitch(kJemaAccountEnable)) {
      cmdline->AppendSwitch(kJemaAccountEnable);
      need_reset_url = true;
    }
  } else if (account_type == AccountType::GOOGLE) {
    if (cmdline->HasSwitch(kJemaAccountEnable)) {
      cmdline->RemoveSwitch(kJemaAccountEnable);
      need_reset_url = true;
    }
  } else {
    return;
  }

  if (need_reset_url) {
    ResetUrls();
  }
}

// Retrieves the active user internally
user_manager::User* GetActiveUserInternal() {
  if (!user_manager::UserManager::IsInitialized()) { return nullptr; }
  user_manager::UserManager* user_manager_ = user_manager::UserManager::Get();
  if (!user_manager_) { return nullptr; }
  user_manager::User* user = user_manager_->GetActiveUser();
  return user;
}

}  // namespace

// Toggles the Jema account flag by account ID
void ToggleJemaAccountFlagByAccountId(const AccountId& account_id) {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  ToggleJemaAccountFlagInternal(command_line, account_id.GetAccountType());
}

// Toggles the Jema account flag for the current command line
void ToggleJemaAccountFlagForCommandLine(base::CommandLine* command_line) {
  user_manager::User* user = GetActiveUserInternal();
  if (!user) return;
  AccountId account_id = user->GetAccountId();
  ToggleJemaAccountFlagInternal(command_line, account_id.GetAccountType());
}

// Toggles the Jema account flag for the command line by account ID
void ToggleJemaAccountFlagForCommandLineByAccountId(base::CommandLine* command_line, const AccountId& account_id) {
  ToggleJemaAccountFlagInternal(command_line, account_id.GetAccountType());
}

// Enables the Jema account flag
void EnableJemaAccountFlag() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  ToggleJemaAccountFlagInternal(command_line, AccountType::JEMA_ACCOUNT);
}

// Disables the Jema account flag
void DisableJemaAccountFlag() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  ToggleJemaAccountFlagInternal(command_line, AccountType::GOOGLE);
}

// Toggles the Jema account flag by the active user
void ToggleJemaAccountFlagByActiveUser() {
  user_manager::User* user = GetActiveUserInternal();
  if (!user) return;
  ToggleJemaAccountFlagByAccountId(user->GetAccountId());
}

// Enables the Jema account flag for a managed device
void EnableJemaAccountFlagForManagedDevice() {
  EnableJemaAccountFlag();
  JemaSetDeviceManagedFlag(true);
}

// Disables the Jema account flag for a managed device
void DisableJemaAccountFlagForManagedDevice() {
  DisableJemaAccountFlag();
  JemaSetDeviceManagedFlag(true);
}

// Appends account switches if needed
void AppendAccountSwitchesIfNeed(const AccountId& account_id, std::vector<std::string>* switches) {
  base::CommandLine cmd_line(base::CommandLine::NO_PROGRAM);
  cmd_line.AppendSwitch(kJemaAccountEnable);
  const std::string account_switch = cmd_line.argv()[1];

  if (account_id.GetAccountType() == AccountType::JEMA_ACCOUNT && std::find(switches->begin(), switches->end(), account_switch) == switches->end()) {
    switches->push_back(account_switch);
  }
}

}  // namespace switches
}  // namespace jemaos