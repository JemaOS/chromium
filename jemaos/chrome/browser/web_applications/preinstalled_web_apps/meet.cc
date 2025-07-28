// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/chrome/browser/web_applications/preinstalled_web_apps/meet.h"

#include "chrome/browser/web_applications/mojom/user_display_mode.mojom.h"
#include "jemaos/switches/urls/urls_constants.h"

namespace web_app {

ExternalInstallOptions GetConfigForGoogleMeet() {
  ExternalInstallOptions options(
      /*install_url=*/GURL(jemaos::constants::kGoogleMeetURL),
      /*user_display_mode=*/mojom::UserDisplayMode::kStandalone,
      /*install_source=*/ExternalInstallSource::kExternalDefault);

  // Support all user types
  options.user_type_allowlist = {"unmanaged", "managed", "child"};
  options.add_to_quick_launch_bar = true;

  return options;
}

}  // namespace web_app