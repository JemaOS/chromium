// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_ACCOUNT_CONSTANTS_H_
#define CHROMEOS_JEMAOS_SWITCHES_ACCOUNT_CONSTANTS_H_

#include <string>
#include "chromeos/chromeos_export.h"
#include "jemaos/build/config/buildflags.h"

namespace jemaos {
namespace constants {

// URLs for JemaOS services
CHROMEOS_EXPORT extern const char kDefaultJemaOSGaiaUrl[];
CHROMEOS_EXPORT extern const char kDefaultJemaOSApisBaseUrl[];
CHROMEOS_EXPORT extern const char kDefaultJemaOSDeviceManagementServerUrl[];
CHROMEOS_EXPORT extern const char kDefaultJemaOSRealtimeReportingServerUrl[];
CHROMEOS_EXPORT extern const char kDefaultJemaOSEncryptedReportingServerUrl[];
CHROMEOS_EXPORT extern const char kJemaOSSyncDevServerUrl[];
CHROMEOS_EXPORT extern const char kJemaOSSyncServerUrl[];

CHROMEOS_EXPORT extern const char kDefaultJemaOSFamilyLinkApisUrl[];

CHROMEOS_EXPORT extern const char kDefaultJemaFtlServerEndpoint[];
CHROMEOS_EXPORT extern const char kDefaultJemaRemotingServerEndpoint[];

// Default sync interval for supervised user settings in seconds
extern const size_t kJemaOSSupervisedUserSettingsDefaultSyncIntervalInSeconds;

// Path to the enrollment token file
CHROMEOS_EXPORT extern const char kJemaEnrollmentTokenFilePath[];

#if BUILDFLAG(IS_OPENJEMA)
// URL for developers to learn how to get API keys
CHROMEOS_EXPORT extern const char kJemaAPIKeysDevelopersHowToURL[];
#endif

}  // namespace constants
}  // namespace jemaos

#endif  // CHROMEOS_JEMAOS_SWITCHES_ACCOUNT_CONSTANTS_H_