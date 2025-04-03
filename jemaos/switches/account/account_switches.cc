// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/account/account_switches.h"
#include "jemaos/switches/account/account_constants.h"
#include "base/command_line.h"

namespace jemaos {
namespace switches {

namespace {

// Command-line switches for JemaOS services
const char kJemaOSFamilyLinkApisUrl[] = "jemaos-family-link-apis-url";
const char kJemaOSSupervisedUserSettingsSyncIntervalInSeconds[] = "jemaos-supervised-user-settings-sync-interval";

const char kPolicyManagedByJema[] = "policy-managed-by-jema";

const char kJemaFtlServerEndpointSwitch[] = "jema-ftl-server-endpoint";
const char kJemaRemotingServerEndpointSwitch[] = "jema-remoting-server-endpoint";

}  // namespace

// Checks if the Jema account is enabled
bool IsJemaAccountEnabled() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  return command_line->HasSwitch(kJemaAccountEnable) && !command_line->HasSwitch(kJemaAccountForceDisabledForTest);
}

// Command-line switches for enabling or disabling Jema accounts
const char kJemaAccountEnable[] = "jema-account-enabled";
const char kJemaAccountForceDisabledForTest[] = "jema-account-force-disabled";

// Checks if the Jema extended account is enabled
bool IsJemaExtendAccountEnabled() {
  return IsJemaAccountEnabled();
}

// Command-line switches for JemaOS service URLs
const char kJemaOSGaiaUrl[] = "jemaos-gaia-url";
const char kJemaOSApisUrl[] = "jemaos-apis-url";
const char kJemaOSDeviceManagementUrl[] = "jemaos-device-management-url";
const char kJemaOSRealtimeReportingUrl[] = "jemaos-realtime-reporting-url";
const char kJemaOSEncryptedReportingUrl[] = "jemaos-encrypted-reporting-url";
const char kJemaOSSyncServiceURL[] = "jemaos-sync-url";

// Retrieves the base URL for JemaOS Kids Management API
std::string GetJemaOSKidsManagementAPIBaseUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSFamilyLinkApisUrl)) {
    return command_line->GetSwitchValueASCII(kJemaOSFamilyLinkApisUrl);
  } else {
    return std::string(jemaos::constants::kDefaultJemaOSFamilyLinkApisUrl);
  }
}

// Retrieves the sync interval for supervised user settings
int GetJemaOSSupervisedUserSettingsSyncInterval() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  int interval = 0;
  if (command_line->HasSwitch(kJemaOSSupervisedUserSettingsSyncIntervalInSeconds)) {
    std::string str = command_line->GetSwitchValueASCII(kJemaOSSupervisedUserSettingsSyncIntervalInSeconds);
    interval = std::stoi(str);
  }
  return interval < 10 ? jemaos::constants::kJemaOSSupervisedUserSettingsDefaultSyncIntervalInSeconds : interval;
}

// Checks if the policy is managed by Jema
bool IsPolicyManagedByJema() {
  return base::CommandLine::ForCurrentProcess()->HasSwitch(kPolicyManagedByJema) || IsJemaAccountEnabled();
}

// Checks if the given URL matches the JemaOS Device Management Server URL
bool IsJemaDMServerUrl(const std::string& url) {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(jemaos::switches::kJemaOSDeviceManagementUrl)) {
    return url == command_line->GetSwitchValueASCII(jemaos::switches::kJemaOSDeviceManagementUrl);
  } else {
    return url == jemaos::constants::kDefaultJemaOSDeviceManagementServerUrl;
  }
}

// Retrieves the Jema FTL server endpoint
std::string GetJemaFtlServerEndpoint() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(jemaos::switches::kJemaFtlServerEndpointSwitch)) {
    return command_line->GetSwitchValueASCII(jemaos::switches::kJemaFtlServerEndpointSwitch);
  } else {
    return jemaos::constants::kDefaultJemaFtlServerEndpoint;
  }
}

// Retrieves the Jema remoting server endpoint
std::string GetJemaRemotingServerEndpoint() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(jemaos::switches::kJemaRemotingServerEndpointSwitch)) {
    return command_line->GetSwitchValueASCII(jemaos::switches::kJemaRemotingServerEndpointSwitch);
  } else {
    return jemaos::constants::kDefaultJemaRemotingServerEndpoint;
  }
}

}  // namespace switches
}  // namespace jemaos