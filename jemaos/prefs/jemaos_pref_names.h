// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_PREF_NAMES_H_
#define CHROMEOS_JEMAOS_PREF_NAMES_H_

namespace jemaos {
namespace prefs {

// Preference to hide the JemaOS Store icon
extern const char kPrefHideJemaOSStoreIcon[];  // Registered in chrome/browser/ui/browser_ui_prefs.cc

// Preference for enabling the JemaOS Improvement Plan
extern const char kJemaOSImprovementPlanEnabled[];

// Local state preferences
extern const char kCurrentEnableArcIMEGlobally[];
extern const char kEnableArcIMEGlobally[];

extern const char kForceTpmFallbackNecessary[];
extern const char kCurrentForceTpmFallback[];
extern const char kForceTpmFallback[];

// Preferences for UI buttons
extern const char kShowSwitchTabletLaptopButton[];
extern const char kShowRebootButtonInTray[];
extern const char kShowRotateScreenButton[];

// Preferences for offline auto sign-in
extern const char kOfflineAutoSigninAccountIdKey[];
extern const char kOfflineAutoSigninPassword[];
extern const char kOfflineAutoSigninPasswordFormat[];
extern const char kOfflineAutoSigninIsChromeLastSignout[];

// Preference for factory reset requests
// Same as kFactoryResetRequested in chrome/common/pref_names.cc
// Resolves dependency conflict issues
extern const char kFactoryResetRequested[];

// Preference for reboot required for Widevine
extern const char kRebootRequiredForWidevine[];

}  // namespace prefs
}  // namespace jemaos

#endif  // CHROMEOS_JEMAOS_PREF_NAMES_H_