# ![Logo](chrome/app/theme/chromium/product_logo_64.png) Chromium

Chromium is an open-source browser project that aims to build a safer, faster,
and more stable way for all users to experience the web.

The project's web site is https://www.chromium.org.

To check out the source code locally, don't use `git clone`! Instead,
follow [the instructions on how to get the code](docs/get_the_code.md).

Documentation in the source is rooted in [docs/README.md](docs/README.md).

Learn how to [Get Around the Chromium Source Code Directory Structure
](https://www.chromium.org/developers/how-tos/getting-around-the-chrome-source-code).

For historical reasons, there are some small top level directories. Now the
guidance is that new top level directories are for product (e.g. Chrome,
Android WebView, Ash). Even if these products have multiple executables, the
code should be in subdirectories of the product.

If you found a bug, please file it at https://crbug.com/new.



JemaOS PWA Integration & Thot Browser Customization
Complete modification summary for JemaOS PWA system and browser branding.

Features
8 Pre-installed PWAs with immediate startup installation
Complete Thot branding replacing Chromium across 73+ languages
Menu-only PWAs (WhatsApp, Teams, Office 365, Miro, Figma, Photopea)
Shelf-pinnable PWAs (Gmail, YouTube)
PWA Implementation
New PWAs Added
| PWA | URL | Menu | Shelf |
|-----|-----|------|-------|
| WhatsApp Web | https://web.whatsapp.com/ | Yes | No |
| Microsoft Teams | https://teams.microsoft.com/ | Yes | No |
| Office 365 | https://www.office.com/ | Yes | No |
| Miro | https://miro.com/ | Yes | No |
| Figma | https://www.figma.com/ | Yes | No |
| Photopea | https://www.photopea.com/ | Yes | No |

Installation System
Immediate installation at first boot (no logout/login required)
Universal deployment for all user types
Automatic menu integration with search functionality
Browser Branding
Complete Chromium to Thot Replacement
About pages: "About Chromium" → "About Thot"
Menu items: "À propos de Chromium" → "À propos de Thot"
Copyright: "The Chromium Authors" → "The Jema Authors"
Help text: "Get help with Chromium" → "Get help with Thot"
License text: "Chromium is made possible by" → "Thot is made possible by"
73+ international languages fully updated
Technical Implementation
Core System Files
chromium/src/chrome/browser/web_applications/
├── preinstalled_web_app_manager.cc     # Installation timing fix
├── preinstalled_apps.cc                # OPENJEMA_BUILD protection
├── BUILD.gn                            # Build system integration
└── preinstalled_web_apps/
    ├── preinstalled_web_apps.cc        # PWA registration
    ├── gmail.cc                        # Menu visibility fix
    └── [new-pwa-files]                 # 6 new PWAs (12 files total)

txt


New Files Created
12 PWA files: 6 headers (.h) + 6 implementations (.cc)
Consistent configuration across all new PWAs
Proper build integration with conditional compilation
URL Corrections
Fixed installation URLs for proper PWA manifest resolution
Standardized www. prefix where required
Maintained Google's special PWA installation URLs
Results
18+ files modified across the codebase
100% PWA installation success at first boot
Complete Thot branding in all languages
Production-ready JemaOS build
Build Configuration
PWA compilation enabled with is_openjema flag:

if (is_openjema) {
  sources += [
    "preinstalled_web_apps/whatsapp.cc",
    "preinstalled_web_apps/teams.cc",
    # ... additional PWAs
  ]
}

txt


This implementation provides a complete PWA ecosystem for JemaOS with custom Thot browser branding.
