// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_ACCOUNT_SWITCHES_H_
#define CHROMEOS_JEMAOS_SWITCHES_ACCOUNT_SWITCHES_H_

#include <string>
#include "chromeos/chromeos_export.h"

namespace jemaos {
namespace switches {

// Checks if the Jema account is enabled
CHROMEOS_EXPORT bool IsJemaAccountEnabled();

// Checks if the Jema extended account is enabled
CHROMEOS_EXPORT bool IsJemaExtendAccountEnabled();

// Command-line switches for enabling or disabling Jema accounts
CHROMEOS_EXPORT extern const char kJemaAccountEnable[];
CHROMEOS_EXPORT extern const char kJemaAccountForceDisabledForTest[];

// Command-line switches for JemaOS service URLs
CHROMEOS_EXPORT extern const char kJemaOSGaiaUrl[];
CHROMEOS_EXPORT extern const char kJemaOSApisUrl[];
CHROMEOS_EXPORT extern const char kJemaOSDeviceManagementUrl[];
CHROMEOS_EXPORT extern const char kJemaOSRealtimeReportingUrl[];
CHROMEOS_EXPORT extern const char kJemaOSEncryptedReportingUrl[];
CHROMEOS_EXPORT extern const char kJemaOSSyncServiceURL[];

// Retrieves the base URL for JemaOS Kids Management API
CHROMEOS_EXPORT std::string GetJemaOSKidsManagementAPIBaseUrl();

// Retrieves the sync interval for supervised user settings
CHROMEOS_EXPORT int GetJemaOSSupervisedUserSettingsSyncInterval();

// Checks if the policy is managed by Jema
CHROMEOS_EXPORT bool IsPolicyManagedByJema();

// Checks if the given URL matches the JemaOS Device Management Server URL
CHROMEOS_EXPORT bool IsJemaDMServerUrl(const std::string& url);

// Retrieves the Jema FTL server endpoint
CHROMEOS_EXPORT std::string GetJemaFtlServerEndpoint();

// Retrieves the Jema remoting server endpoint
CHROMEOS_EXPORT std::string GetJemaRemotingServerEndpoint();

}  // namespace switches
}  // namespace jemaos

#endif  // CHROMEOS_JEMAOS_SWITCHES_ACCOUNT_SWITCHES_H_