// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/account/account_switches.h"
#include "jemaos/switches/account/account_constants.h"
#include "base/command_line.h"

namespace jemaos {
namespace switches {

namespace {

const char kJemaOSFamilyLinkApisUrl[] = "jemaos-family-link-apis-url";
const char kJemaOSSupervisedUserSettingsSyncIntervalInSeconds[] = "jemaos-supervised-user-settings-sync-interval";

const char kPolicyManagedByJema[] = "policy-managed-by-jema";

const char kJemaFtlServerEndpointSwitch[] = "jema-ftl-server-endpoint";
const char kJemaRemotingServerEndpointSwitch[] = "jema-remoting-server-endpoint";
}

bool IsJemaAccountEnabled() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  return command_line->HasSwitch(kJemaAccountEnable) && !command_line->HasSwitch(kJemaAccountForceDisabledForTest);
}

const char kJemaAccountEnable[] = "jema-account-enabled";
const char kJemaAccountForceDisabledForTest[] = "jema-account-force-disabled";

bool IsJemaExtendAccountEnabled() {
  return IsJemaAccountEnabled();
}

const char kJemaOSGaiaUrl[] = "jemaos-gaia-url";
const char kJemaOSApisUrl[] = "jemaos-apis-url";
const char kJemaOSDeviceManagementUrl[] = "jemaos-device-management-url";
const char kJemaOSRealtimeReportingUrl[] = "jemaos-realtime-reporting-url";
const char kJemaOSEncryptedReportingUrl[] = "jemaos-encrypted-reporting-url";
const char kJemaOSSyncServiceURL[] = "jemaos-sync-url";

std::string GetJemaOSKidsManagementAPIBaseUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSFamilyLinkApisUrl)) {
    return command_line->GetSwitchValueASCII(kJemaOSFamilyLinkApisUrl);
  } else {
    return std::string(jemaos::constants::kDefaultJemaOSFamilyLinkApisUrl);
  }
}

int GetJemaOSSupervisedUserSettingsSyncInterval() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  int interval = 0;
  if (command_line->HasSwitch(kJemaOSSupervisedUserSettingsSyncIntervalInSeconds)) {
    std::string str = command_line->GetSwitchValueASCII(kJemaOSSupervisedUserSettingsSyncIntervalInSeconds);
    interval = std::stoi(str);
  }
  return interval < 10 ? jemaos::constants::kJemaOSSupervisedUserSettingsDefaultSyncIntervalInSeconds : interval;
}

bool IsPolicyManagedByJema() {
  // Jema accounts are enterprise-managed through the Jema SaaS (the device
  // management flow works when the DM backend is reachable). The login
  // black screen was caused by the OAuth2 session-restore termination, not
  // by the DM policy path — keep the original behavior.
  return base::CommandLine::ForCurrentProcess()->HasSwitch(
             kPolicyManagedByJema) ||
         IsJemaAccountEnabled();
}

bool IsJemaDMServerUrl(const std::string& url) {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(jemaos::switches::kJemaOSDeviceManagementUrl)) {
    return url == command_line->GetSwitchValueASCII(jemaos::switches::kJemaOSDeviceManagementUrl);
  } else {
    return url == jemaos::constants::kDefaultJemaOSDeviceManagementServerUrl;
  }
}


std::string GetJemaFtlServerEndpoint() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(jemaos::switches::kJemaFtlServerEndpointSwitch)) {
    return command_line->GetSwitchValueASCII(jemaos::switches::kJemaFtlServerEndpointSwitch);
  } else {
    return jemaos::constants::kDefaultJemaFtlServerEndpoint;
  }
}

std::string GetJemaRemotingServerEndpoint() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(jemaos::switches::kJemaRemotingServerEndpointSwitch)) {
    return command_line->GetSwitchValueASCII(jemaos::switches::kJemaRemotingServerEndpointSwitch);
  } else {
    return jemaos::constants::kDefaultJemaRemotingServerEndpoint;
  }
}


}// namespace switches
}// namespace jemaos
