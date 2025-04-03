// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/services/services_switches.h"

#include "base/command_line.h"
#include "chrome/common/chrome_switches.h"
#include "jemaos/switches/services/services_constants.h"
#include "jemaos/switches/urls/urls_constants.h"
#include "jemaos/build/config/buildflags.h"

namespace jemaos {
namespace switches {

namespace {

// Command-line switches for disabling APIs
const char kDisableJemaOSGeolocationAPI[] = "disable-jemaos-geolocation-api";
const char kDisableJemaOSTimezoneAPI[] = "disable-jemaos-timezone-api";

// Command-line switches for custom API URLs
const char kJemaOSGeolocationAPIUrl[] = "jemaos-geolocation-api-url";
const char kJemaOSTimezoneAPIUrl[] = "jemaos-timezone-api-url";

// Command-line switch for Looking Glass URL
const char kJemaOSLookingGlassUrl[] = "jemaos-lookingglass-url";

// Command-line switches for App Store URLs
const char kJemaOSAppsGalleryURL[] = "jemaos-apps-gallery-url";
const char kJemaOSAppsGalleryUpdateURL[] = "jemaos-apps-gallery-update-url";

// Command-line switch for Assistant Web URL
const char kJemaOSAssistantWebUrl[] = "jemaos-ai-url";

// Store URL prefixes
const char kJemaOSStoreComPrefix[] = "https://store.jemaos.com";
const char kJemaOSStoreIoPrefix[] = "https://store.jemaos.io";

}  // namespace

// Checks if the Geolocation API is disabled
bool DisableJemaOSGeolocationAPI() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  return command_line->HasSwitch(kDisableJemaOSGeolocationAPI);
}

// Checks if the Timezone API is disabled
bool DisableJemaOSTimezoneAPI() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  return command_line->HasSwitch(kDisableJemaOSTimezoneAPI);
}

// Retrieves the Geolocation API URL
std::string GetJemaOSGeolocationAPIUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSGeolocationAPIUrl)) {
    return command_line->GetSwitchValueASCII(kJemaOSGeolocationAPIUrl);
  } else {
    return std::string(jemaos::constants::kDefaultJemaOSGeolocationAPIUrl);
  }
}

// Retrieves the Timezone API URL
std::string GetJemaOSTimezoneAPIUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSTimezoneAPIUrl)) {
    return command_line->GetSwitchValueASCII(kJemaOSTimezoneAPIUrl);
  } else {
    return std::string(jemaos::constants::kDefaultJemaOSTimezoneAPIUrl);
  }
}

// Retrieves the Looking Glass URL
std::string GetJemaOSLookingGlassUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSLookingGlassUrl)) {
    return command_line->GetSwitchValueASCII(kJemaOSLookingGlassUrl);
  } else {
    return std::string(jemaos::constants::kDefaultJemaOSLookingGlassUrl);
  }
}

// Retrieves the App Store URL
std::string GetJemaOSAppStoreURL() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSAppsGalleryURL)) {
    return command_line->GetSwitchValueASCII(kJemaOSAppsGalleryURL);
  } else {
    return std::string(jemaos::constants::kJemaOSStoreBaseUrl);
  }
}

// Retrieves the Web Store Update URL
std::string GetJemaOSWebStoreUpdateUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSAppsGalleryUpdateURL)) {
    return command_line->GetSwitchValueASCII(kJemaOSAppsGalleryUpdateURL);
  } else {
    return std::string(jemaos::constants::kJemaOSWebStoreUpdateURL);
  }
}

// Retrieves the Assistant Web URL
std::string GetJemaOSAssistantWebUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSAssistantWebUrl)) {
    std::string value(command_line->GetSwitchValueASCII(kJemaOSAssistantWebUrl));
    if (!value.empty()) {
      return value;
    }
  }
  return std::string(jemaos::constants::kJemaOSAssistantDefaultWebUrl);
}

// Converts the Web Store Update URL based on the domain
CHROMEOS_EXPORT std::string MayConvertWebStoreUpdateUrl(
    const std::string& url) {
  std::string new_url = url;
#if BUILDFLAG(USE_JEMAOS_COM)
  if (new_url.find(kJemaOSStoreIoPrefix) == 0) {
    new_url.replace(0, sizeof(kJemaOSStoreIoPrefix) - 1,
                    kJemaOSStoreComPrefix);
  }
#else
  if (new_url.find(kJemaOSStoreComPrefix) == 0) {
    new_url.replace(0, sizeof(kJemaOSStoreComPrefix) - 1,
                    kJemaOSStoreIoPrefix);
  }
#endif
  return new_url;
}

}  // namespace switches
}  // namespace jemaos