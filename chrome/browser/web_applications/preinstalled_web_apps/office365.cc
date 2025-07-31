// Copyright 2024 The Jema Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/web_applications/preinstalled_web_apps/office365.h"

#include "base/functional/bind.h"
#include "base/strings/utf_string_conversions.h"
#include "build/build_config.h"
#include "chrome/browser/web_applications/mojom/user_display_mode.mojom.h"
#include "chrome/browser/web_applications/preinstalled_app_install_features.h"
#include "chrome/browser/web_applications/preinstalled_web_apps/preinstalled_web_app_definition_utils.h"
#include "chrome/browser/web_applications/web_app_id_constants.h"
#include "chrome/browser/web_applications/web_app_install_info.h"

namespace web_app {

ExternalInstallOptions GetConfigForOffice365() {
  ExternalInstallOptions options(
      /*install_url=*/GURL("https://www.office.com/"),
      /*user_display_mode=*/mojom::UserDisplayMode::kStandalone,
      /*install_source=*/ExternalInstallSource::kExternalDefault);

  options.user_type_allowlist = {"unmanaged", "managed", "child"};
  options.only_for_new_users = false;
  options.override_previous_user_uninstall = true;
  options.add_to_applications_menu = true;
  options.add_to_search = true;
  options.add_to_management = true;
  options.add_to_desktop = false;
  options.add_to_quick_launch_bar = false;  // No shelf pinning

  return options;
}

}  // namespace web_app