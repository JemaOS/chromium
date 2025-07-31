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
Overview
This document outlines the complete modification summary for implementing PWA system and custom browser branding in JemaOS.

Key Features
8 pre-installed PWAs with immediate startup installation
Complete Chromium to Thot branding across 73+ languages
Menu-only PWAs for productivity applications
Shelf-pinnable PWAs for core Google services
PWA Implementation
Added Applications
Menu Only (No Shelf Pinning)

WhatsApp Web - https://web.whatsapp.com/
Microsoft Teams - https://teams.microsoft.com/
Office 365 - https://www.office.com/
Miro - https://miro.com/
Figma - https://www.figma.com/
Photopea - https://www.photopea.com/
Shelf Pinnable

Gmail - Enhanced menu visibility
YouTube - Existing functionality
Installation Features
Immediate installation at first boot
No logout/login cycle required
Universal deployment for all user types
Automatic menu integration with search
Browser Branding Changes
Text Replacements
Page titles: "About Chromium" → "About Thot"
Menu items: "À propos de Chromium" → "À propos de Thot"
Copyright: "The Chromium Authors" → "The Jema Authors"
Help text: "Get help with Chromium" → "Get help with Thot"
License: "Chromium is made possible by" → "Thot is made possible by"
International Support
73+ language files updated
Complete branding consistency across all locales
Technical Implementation
Files Modified
Core System

preinstalled_web_app_manager.cc - Installation timing fix
preinstalled_apps.cc - OPENJEMA_BUILD protection
BUILD.gn - Build system integration
preinstalled_web_apps.cc - PWA registration
gmail.cc - Menu visibility enhancement
New PWA Files

12 files total: 6 headers (.h) + 6 implementations (.cc)
Consistent configuration across all applications
Conditional compilation with is_openjema flag
Build Configuration
if (is_openjema) {
  sources += [
    "preinstalled_web_apps/whatsapp.cc",
    "preinstalled_web_apps/teams.cc",
    "preinstalled_web_apps/office365.cc",
    "preinstalled_web_apps/miro.cc",
    "preinstalled_web_apps/figma.cc",
    "preinstalled_web_apps/photopea.cc",
  ]
}

txt


Results
18+ files modified across codebase
100% PWA installation success at first boot
Complete Thot branding in all supported languages
Production-ready JemaOS build with enhanced user experience
Installation URLs
URL corrections were applied to ensure proper PWA manifest resolution:

Office 365: Added www. prefix
Figma: Added www. prefix
Miro: Simplified to root domain
Photopea: Added www. prefix
Google services maintain their specialized PWA installation URLs.
