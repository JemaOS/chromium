// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/services/services_switches.h"

#include "base/command_line.h"
#include "jemaos/switches/services/services_constants.h"
#include "jemaos/switches/urls/urls_constants.h"
#include "jemaos/build/config/buildflags.h"

namespace jemaos {
namespace switches {

namespace {

const char kDisableJemaOSGeolocationAPI[] = "disable-jemaos-geolocation-api";
const char kDisableJemaOSTimezoneAPI[] = "disable-jemaos-timezone-api";

const char kJemaOSGeolocationAPIUrl[] = "jemaos-geolocation-api-url";
const char kJemaOSTimezoneAPIUrl[] = "jemaos-timezone-api-url";

const char kJemaOSLookingGlassUrl[] = "jemaos-lookingglass-url";

const char kJemaOSAppsGalleryURL[] = "jemaos-apps-gallery-url";

const char kJemaOSAppsGalleryUpdateURL[] = "jemaos-apps-gallery-update-url";

const char kJemaOSAssistantWebUrl[] = "jemaos-ai-url";

const char kJemaOSStoreComPrefix[] = "https://store.jemaos.com";
const char kJemaOSStoreIoPrefix[] = "https://chromewebstore.google.com";

#if BUILDFLAG(JEMAOS_DEVICE)
const char kJemaOSProductWarrantyURL[] = "jemaos-product-warranty-url";
#endif

}

bool DisableJemaOSGeolocationAPI() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  return command_line->HasSwitch(kDisableJemaOSGeolocationAPI);
}

bool DisableJemaOSTimezoneAPI() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  return command_line->HasSwitch(kDisableJemaOSTimezoneAPI);
}

std::string GetJemaOSGeolocationAPIUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSGeolocationAPIUrl)) {
    return command_line->GetSwitchValueASCII(kJemaOSGeolocationAPIUrl);
  } else {
    return std::string(jemaos::constants::kDefaultJemaOSGeolocationAPIUrl);
  }
}

std::string GetJemaOSTimezoneAPIUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSTimezoneAPIUrl)) {
    return command_line->GetSwitchValueASCII(kJemaOSTimezoneAPIUrl);
  } else {
    return std::string(jemaos::constants::kDefaultJemaOSTimezoneAPIUrl);
  }
}

std::string GetJemaOSLookingGlassUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSLookingGlassUrl)) {
    return command_line->GetSwitchValueASCII(kJemaOSLookingGlassUrl);
  } else {
    return std::string(jemaos::constants::kDefaultJemaOSLookingGlassUrl);
  }
}

std::string GetJemaOSAppStoreURL() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSAppsGalleryURL)) {
    return command_line->GetSwitchValueASCII(kJemaOSAppsGalleryURL);
  } else {
    return std::string(jemaos::constants::kJemaOSStoreBaseUrl);
  }
}

std::string GetJemaOSWebStoreUpdateUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSAppsGalleryUpdateURL)) {
    return command_line->GetSwitchValueASCII(kJemaOSAppsGalleryUpdateURL);
  } else {
    return std::string(jemaos::constants::kJemaOSWebStoreUpdateURL);
  }
}

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

#if BUILDFLAG(JEMAOS_DEVICE)
std::string GetJemaOSProductWarrantyUrl() {
  base::CommandLine* command_line = base::CommandLine::ForCurrentProcess();
  if (command_line->HasSwitch(kJemaOSProductWarrantyURL)) {
    return command_line->GetSwitchValueASCII(kJemaOSProductWarrantyURL);
  } else {
    return std::string(jemaos::constants::kJemaOSProductWarrantyDefaultURL);
  }
}
#endif

} // switches
} // jemaos
