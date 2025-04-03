// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/prefs/jemaos_pref_names.h"

namespace jemaos {
namespace prefs {

// Preference to hide the JemaOS Store icon
const char kPrefHideJemaOSStoreIcon[] = "hide_jemaos_store_icon";

// Preference for enabling the JemaOS Improvement Plan
const char kJemaOSImprovementPlanEnabled[] = "jemaos_improvement_plan_enabled";

// Local state preferences
const char kEnableArcIMEGlobally[] = "enable_arc_ime_globally";
const char kCurrentEnableArcIMEGlobally[] = "current_enable_arc_ime_globally";

const char kForceTpmFallbackNecessary[] = "force_tpm_fallback_necessary";
const char kCurrentForceTpmFallback[] = "current_force_tpm_fallback";
const char kForceTpmFallback[] = "force_tpm_fallback";

// Preferences for UI buttons
const char kShowSwitchTabletLaptopButton[] = "show_switch_tablet_laptop_button";
const char kShowRebootButtonInTray[] = "show_reboot_button_in_tray";
const char kShowRotateScreenButton[] = "show_rotate_screen_button";

// Preferences for offline auto sign-in
const char kOfflineAutoSigninAccountIdKey[] = "offline_auto_signin.account_id_key";
const char kOfflineAutoSigninPassword[] = "offline_auto_signin.password";
const char kOfflineAutoSigninPasswordFormat[] = "offline_auto_signin.password_format";
const char kOfflineAutoSigninIsChromeLastSignout[] = "offline_auto_signin.chrome_signout";

// Preference for factory reset requests
const char kFactoryResetRequested[] = "FactoryResetRequested";

// Preference for reboot required for Widevine
const char kRebootRequiredForWidevine[] = "reboot_required_for_widevine";

}  // namespace prefs
}  // namespace jemaos