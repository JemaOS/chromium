// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/account/account_constants.h"
#include "jemaos/build/config/buildflags.h"

namespace jemaos::constants {

#if BUILDFLAG(USE_JEMAOS_COM)

// URLs for JemaOS services when using the .com domain
const char kDefaultJemaOSGaiaUrl[] = "https://account.jemaos.com";
const char kDefaultJemaOSApisBaseUrl[] = "https://apis.jemaos.com";
const char kDefaultJemaOSDeviceManagementServerUrl[] = "https://policy.jemaos.com";
const char kDefaultJemaOSRealtimeReportingServerUrl[] = "https://apis.jemaos.com/report/events";
const char kDefaultJemaOSEncryptedReportingServerUrl[] = "https://apis.jemaos.com/report/record";
const char kJemaOSSyncDevServerUrl[] = "https://clients4.jemaos.com/chrome-sync/dev";
const char kJemaOSSyncServerUrl[] = "https://clients4.jemaos.com/chrome-sync";
const char kDefaultJemaOSFamilyLinkApisUrl[] = "https://familylink-apis.jemaos.com/kidsmanagement/v1/";

const char kDefaultJemaFtlServerEndpoint[] = "im.jemaos.com";
const char kDefaultJemaRemotingServerEndpoint[] = "remoting.jemaos.com";

#else

// URLs for JemaOS services when using the .io domain
const char kDefaultJemaOSGaiaUrl[] = "https://account.jemaos.io";
const char kDefaultJemaOSApisBaseUrl[] = "https://apis.jemaos.io";
const char kDefaultJemaOSDeviceManagementServerUrl[] = "https://policy.jemaos.io";
const char kDefaultJemaOSRealtimeReportingServerUrl[] = "https://apis.jemaos.io/report/events";
const char kDefaultJemaOSEncryptedReportingServerUrl[] = "https://apis.jemaos.io/report/record";
const char kJemaOSSyncDevServerUrl[] = "https://clients4.jemaos.io/chrome-sync/dev";
const char kJemaOSSyncServerUrl[] = "https://clients4.jemaos.io/chrome-sync";
const char kDefaultJemaOSFamilyLinkApisUrl[] = "https://familylink-apis.jemaos.io/kidsmanagement/v1/";

const char kDefaultJemaFtlServerEndpoint[] = "im.jemaos.io";
const char kDefaultJemaRemotingServerEndpoint[] = "remoting.jemaos.io";

#endif

// Default sync interval for supervised user settings in seconds
const size_t kJemaOSSupervisedUserSettingsDefaultSyncIntervalInSeconds = 600;

// Path to the enrollment token file
const char kJemaEnrollmentTokenFilePath[] = "/usr/share/oem/jemaos_enroll_token";

#if BUILDFLAG(IS_OPENJEMA)
// URL for developers to learn how to get API keys
const char kJemaAPIKeysDevelopersHowToURL[] = "https://jematechnology.fr/docs/developers/how-to-get-api-keys";
#endif

}  // namespace jemaos::constants