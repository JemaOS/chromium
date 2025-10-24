// Copyright (c) 2021 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_PREF_NAMES_H_
#define CHROMEOS_JEMAOS_PREF_NAMES_H_

#include "jemaos/build/config/buildflags.h"

namespace jemaos {
namespace prefs {


extern const char kPrefHideJemaOSStoreIcon[]; // registered in chrome/browser/ui/browser_ui_prefs.cc

extern const char kJemaOSImprovementPlanEnabled[];

// local state
extern const char kCurrentEnableArcIMEGlobally[];
extern const char kEnableArcIMEGlobally[];

extern const char kForceTpmFallbackNecessary[];
extern const char kCurrentForceTpmFallback[];
extern const char kForceTpmFallback[];

extern const char kShowSwitchTabletLaptopButton[];
extern const char kShowRebootButtonInTray[];
extern const char kShowRotateScreenButton[];

extern const char kOfflineAutoSigninAccountIdKey[];
extern const char kOfflineAutoSigninPassword[];
extern const char kOfflineAutoSigninPasswordFormat[];
extern const char kOfflineAutoSigninIsChromeLastSignout[];

// same with kFactoryResetRequested in chrome/common/pref_names.cc
// dep conflict issue
extern const char kFactoryResetRequested[];

extern const char kRebootRequiredForWidevine[];

extern const char kJemaAssistantEnabled[];
extern const char kJemaAssistantExtraAcceleratorEnabled[];

extern const char kJemaOSArcMediaAutoScanEnabled[];

#if BUILDFLAG(USE_JEMAOS_LICENSE)
extern const char kJemaLicenseShouldShowInSettings[];
extern const char kJemaLicenseStateType[];
extern const char kJemaLicenseEnforcementLevel[];
extern const char kJemaLicenseEnforcementLogOutInterval[];
#endif


#if BUILDFLAG(USE_JEMAOS_COM)
extern const char kCrostiniInstallerNotificationUserInteracted[];
#endif

} // prefs
} // jemaos

#endif
