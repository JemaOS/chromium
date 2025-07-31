// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/functional/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "chrome/browser/web_applications/mojom/user_display_mode.mojom.h"
#include "chrome/browser/web_applications/preinstalled_app_install_features.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/google_docs.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/preinstalled_web_app_definition_utils.h"
#include "chrome/browser/web_applications/web_app_id_constants.h"
#include "chrome/browser/web_applications/web_app_install_info.h"

namespace web_app {


ExternalInstallOptions GetConfigForGoogleCalendar() {
  ExternalInstallOptions options(
      /*install_url=*/GURL("https://calendar.google.com/calendar/"
                           "installwebapp?usp=chrome_default"),
      /*user_display_mode=*/mojom::UserDisplayMode::kStandalone,
      /*install_source=*/ExternalInstallSource::kExternalDefault);

  options.user_type_allowlist = {"unmanaged", "managed", "child"};
  options.uninstall_and_replace.push_back("ejjicmeblgpmajnghnpcppodonldlgfn");
  options.disable_if_tablet_form_factor = true;
  options.expected_app_id = kGoogleCalendarAppId;

  return options;
}

}  // namespace web_app
