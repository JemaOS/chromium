// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/web_applications/preinstalled_web_apps/setsound.h"

namespace web_app {

ExternalInstallOptions GetConfigForSetSound() {
  ExternalInstallOptions options(
      /*install_url=*/GURL("https://setsound.jemaos.com/"),
      /*user_display_mode=*/mojom::UserDisplayMode::kStandalone,
      /*install_source=*/ExternalInstallSource::kExternalDefault);

  options.user_type_allowlist = {"unmanaged", "managed", "child"};
  options.add_to_quick_launch_bar = false;
  options.oem_installed = true;

  return options;
}

}  // namespace web_app
