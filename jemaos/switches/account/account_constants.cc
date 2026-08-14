// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/account/account_constants.h"
#include "jemaos/build/config/buildflags.h"

namespace jemaos::constants {

#if BUILDFLAG(USE_JEMAOS_COM)

const char kDefaultJemaOSGaiaUrl[] = "https://8ab03d25-4808-4e5e-bf86-c9739f5b65eb.auth-app.jematech.fr";
const char kDefaultJemaOSApisBaseUrl[] = "https://8ab03d25-4808-4e5e-bf86-c9739f5b65eb.auth-app.jematech.fr";
const char kDefaultJemaOSDeviceManagementServerUrl[] = "https://8ab03d25-4808-4e5e-bf86-c9739f5b65eb.auth-app.jematech.fr/devicemanagement";
const char kDefaultJemaOSRealtimeReportingServerUrl[] =
  "https://apis.jemaos.com/report/events";
const char kDefaultJemaOSEncryptedReportingServerUrl[] =
  "https://apis.jemaos.com/report/record";
const char kJemaOSSyncDevServerUrl[] = "https://8ab03d25-4808-4e5e-bf86-c9739f5b65eb.auth-app.jematech.fr/chrome-sync";
const char kJemaOSSyncServerUrl[] = "https://8ab03d25-4808-4e5e-bf86-c9739f5b65eb.auth-app.jematech.fr/chrome-sync";
const char kDefaultJemaOSFamilyLinkApisUrl[] =
  "https://familylink-apis.jemaos.com/kidsmanagement/v1/";

const char kDefaultJemaFtlServerEndpoint[] = "im.jemaos.com";
const char kDefaultJemaRemotingServerEndpoint[] = "remoting.jemaos.com";

#else

const char kDefaultJemaOSGaiaUrl[] = "https://8ab03d25-4808-4e5e-bf86-c9739f5b65eb.auth-app.jematech.fr";
const char kDefaultJemaOSApisBaseUrl[] = "https://apis.jematechnology.fr";
const char kDefaultJemaOSDeviceManagementServerUrl[] =
  "https://policy.jematechnology.fr";
const char kDefaultJemaOSRealtimeReportingServerUrl[] =
  "https://apis.jematechnology.fr/report/events";
const char kDefaultJemaOSEncryptedReportingServerUrl[] =
  "https://apis.jematechnology.fr/report/record";
const char kJemaOSSyncDevServerUrl[] =
  "https://clients4.jematechnology.fr/chrome-sync/dev";
const char kJemaOSSyncServerUrl[] = "https://clients4.jematechnology.fr/chrome-sync";
const char kDefaultJemaOSFamilyLinkApisUrl[] =
  "https://familylink-apis.jematechnology.fr/kidsmanagement/v1/";

const char kDefaultJemaFtlServerEndpoint[] = "im.jematechnology.fr";
const char kDefaultJemaRemotingServerEndpoint[] = "remoting.jematechnology.fr";

#endif

const size_t kJemaOSSupervisedUserSettingsDefaultSyncIntervalInSeconds = 600;
const char kJemaEnrollmentTokenFilePath[] =
  "/usr/share/oem/jemaos_enroll_token";

#if BUILDFLAG(IS_OPENJEMA)
const char kJemaAPIKeysDevelopersHowToURL[] =
  "https://openjema.com/docs/developers/how-to-get-api-keys";
#endif

}  // namespace jemaos::constants
