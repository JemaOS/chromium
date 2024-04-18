// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_ACCOUNT_SWITCHES_H_
#define CHROMEOS_JEMAOS_SWITCHES_ACCOUNT_SWITCHES_H_

#include <string>
#include "chromeos/chromeos_export.h"

namespace jemaos {
namespace switches {

CHROMEOS_EXPORT bool IsJemaAccountEnabled();
CHROMEOS_EXPORT bool IsJemaExtendAccountEnabled();

CHROMEOS_EXPORT extern const char kJemaAccountEnable[];
CHROMEOS_EXPORT extern const char kJemaAccountForceDisabledForTest[];

CHROMEOS_EXPORT extern const char kJemaOSGaiaUrl[];
CHROMEOS_EXPORT extern const char kJemaOSApisUrl[];
CHROMEOS_EXPORT extern const char kJemaOSDeviceManagementUrl[];
CHROMEOS_EXPORT extern const char kJemaOSRealtimeReportingUrl[];
CHROMEOS_EXPORT extern const char kJemaOSEncryptedReportingUrl[];
CHROMEOS_EXPORT extern const char kJemaOSSyncServiceURL[];
extern std::string GetJemaOSKidsManagementAPIBaseUrl();
CHROMEOS_EXPORT int GetJemaOSSupervisedUserSettingsSyncInterval();
CHROMEOS_EXPORT bool IsPolicyManagedByJema();

bool IsJemaDMServerUrl(const std::string& url);

CHROMEOS_EXPORT std::string GetJemaFtlServerEndpoint();
CHROMEOS_EXPORT std::string GetJemaRemotingServerEndpoint();

}
}

#endif
