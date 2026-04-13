# JemaOS Preinstalled Web Apps (PWAs)

This directory contains the C++ configuration files for preinstalled PWAs in JemaOS.

## Overview

JemaOS uses the hardcoded C++ approach to preinstall PWAs. This is the most robust method, as it compiles the PWA configurations directly into Chrome, ensuring they are always installed on first boot regardless of network conditions or policy settings.

## How It Works

1. Each PWA has a header file (`.h`) and implementation file (`.cc`)
2. The implementation file defines a `GetConfigFor*()` function that returns an `ExternalInstallOptions` struct
3. These functions are called from `chrome/browser/web_applications/preinstalled_web_apps/preinstalled_web_apps.cc`
4. The source files are compiled via `chrome/browser/web_applications/BUILD.gn`

## Preinstalled PWAs

| App | URL | Function | Folder |
|-----|-----|----------|--------|
| JemaOS Community | https://community.jemaos.io/ | `GetConfigForJemaCommunity()` | Root |
| JemaOS Remote Desktop | https://jema-rdp.vercel.app/ | `GetConfigForJemaRemoteDesktop()` | Root |
| JemaOS Notes | https://jemanote-pwa.vercel.app/ | `GetConfigForJemaNotes()` | **OEM/JEMA** |
| Mistral AI | https://chat.mistral.ai/chat | `GetConfigForMistral()` | Root |
| Qwant | https://www.qwant.com/?l=fr | `GetConfigForQwant()` | Root |
| Calculator | https://calculator.apps.chrome/ | `GetConfigForJemaCalculator()` | Root |
| Galerie | https://galerie-eta.vercel.app/ | `GetConfigForGalerie()` | Root |
| Osivibe | https://osivibe.vercel.app/ | `GetConfigForOsivibe()` | **OEM/JEMA** |
| Anu (Nephtys) | https://anu-nine.vercel.app/ | `GetConfigForAnu()` | **OEM/JEMA** |
| JemaChess | https://jemachess.vercel.app/ | `GetConfigForJemaChess()` | **OEM/JEMA** |
| SetSound | https://setsound.vercel.app/ | `GetConfigForSetSound()` | **OEM/JEMA** |
| Anima | https://anima-app-one.vercel.app/ | `GetConfigForAnima()` | **OEM/JEMA** |
| Gmail | https://mail.google.com/ | `GetConfigForJemaGmail()` | Root |
| Text Editor | https://text.app/ | `GetConfigForJemaText()` | Root |
| Office 365 | https://www.office.com/?auth=1 | `GetConfigForOffice365()` | Root |
| ToffeeShare | https://toffeeshare.com/ | `GetConfigForToffeeShare()` | Root |
| Excalidraw | https://excalidraw.com/ | `GetConfigForExcalidraw()` | Root |
| VS Code | https://vscode.dev/ | `GetConfigForVSCode()` | Root |
| WhatsApp | https://web.whatsapp.com/ | `GetConfigForWhatsApp()` | Root |
| Teams | https://teams.microsoft.com/ | `GetConfigForTeams()` | Root |
| Google Docs | https://docs.google.com/ | `GetConfigForJemaDocs()` | Root |
| Google Sheets | https://docs.google.com/spreadsheets | `GetConfigForJemaSheets()` | Root |
| Google Meet | https://meet.google.com/ | `GetConfigForJemaMeet()` | Root |

## Adding a New PWA

### Step 1: Create Header File

Create `newapp.h` in `chrome/browser/web_applications/preinstalled_web_apps/`:

```cpp
// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_WEB_APPLICATIONS_PREINSTALLED_WEB_APPS_NEWAPP_H_
#define CHROME_BROWSER_WEB_APPLICATIONS_PREINSTALLED_WEB_APPS_NEWAPP_H_

#include "chrome/browser/web_applications/external_install_options.h"

namespace web_app {

ExternalInstallOptions GetConfigForNewApp();

}  // namespace web_app

#endif  // CHROME_BROWSER_WEB_APPLICATIONS_PREINSTALLED_WEB_APPS_NEWAPP_H_
```

### Step 2: Create Implementation File

Create `newapp.cc` in `chrome/browser/web_applications/preinstalled_web_apps/`:

```cpp
// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/web_applications/preinstalled_web_apps/newapp.h"

namespace web_app {

ExternalInstallOptions GetConfigForNewApp() {
  ExternalInstallOptions options(
      /*install_url=*/GURL("https://example.com/"),
      /*user_display_mode=*/mojom::UserDisplayMode::kStandalone,
      /*install_source=*/ExternalInstallSource::kExternalDefault);

  options.user_type_allowlist = {"unmanaged", "managed", "child"};
  options.add_to_quick_launch_bar = false;
  
  // Set to true to put in OEM folder, false for Root
  options.oem_installed = true; 

  return options;
}

}  // namespace web_app
```

### Step 3: Update preinstalled_web_apps.cc

In `chrome/browser/web_applications/preinstalled_web_apps/preinstalled_web_apps.cc`:

1. Add the include at the top:
```cpp
#include "chrome/browser/web_applications/preinstalled_web_apps/newapp.h"
```

2. Add the function call in `GetChromeBrandedApps` (for branded builds) AND `GetPreinstalledWebApps` (for non-branded builds):
```cpp
// In GetChromeBrandedApps:
return {
  // ...
  GetConfigForNewApp(),
};

// In GetPreinstalledWebApps (#else block):
return {
  // ...
  GetConfigForNewApp(),
};
```

### Step 4: Update BUILD.gn

In `chrome/browser/web_applications/BUILD.gn`, add the source files:

```gn
sources += [
  // ... other JemaOS files ...
  "preinstalled_web_apps/newapp.cc",
  "preinstalled_web_apps/newapp.h",
]
```

### Step 5: Rebuild Chrome

```bash
cros_sdk
emerge-amd64-jemaos chromeos-chrome
```
