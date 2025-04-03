// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_PREFS_H_
#define CHROMEOS_JEMAOS_PREFS_H_

#include "jemaos/prefs/jemaos_pref_names.h"

class PrefRegistrySimple;
class PrefService;

namespace jemaos {

namespace prefs {

// Registers local state preferences
void RegisterLocalStatePrefs(PrefRegistrySimple* registry);

// Registers profile-specific preferences
void RegisterProfilePrefs(PrefRegistrySimple* registry);

// Keeps the current preferences in sync with their corresponding values
void KeepCurrentPrefs(PrefService* local_state);

// Marks TPM fallback as not necessary
void SetNotNecessaryForceTpmFallback(PrefService* local_state);

// Clears the reboot mark preferences
void ClearRebootMarkPrefs(PrefService* local_state);

}  // namespace prefs

}  // namespace jemaos

#endif  // CHROMEOS_JEMAOS_PREFS_H_