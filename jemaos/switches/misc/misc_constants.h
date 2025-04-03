// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_MISC_CONSTANTS_H_
#define CHROMEOS_JEMAOS_SWITCHES_MISC_CONSTANTS_H_

#include "chromeos/chromeos_export.h"

namespace jemaos {
namespace constants {

// Default time zone ID for JemaOS
CHROMEOS_EXPORT extern const char kJemaOSDefaultTimeZoneId[];

// Header for the system information section
CHROMEOS_EXPORT extern const char kJemaOSSystemInfoHeader[];

// Temporary prefix for system information files
CHROMEOS_EXPORT extern const char kJemaOSSystemTempPrefix[];

// File name for exporting system information
CHROMEOS_EXPORT extern const char kJemaOSSystemInfoFileName[];

// Command to retrieve hardware tuner information
CHROMEOS_EXPORT extern const char kJemaOSHwtunerCommand[];

// Section name for DMI information in the hardware tuner output
CHROMEOS_EXPORT extern const char kJemaOSHwtunerInfoSectionName[];

// Base path for JemaOS wallpapers
CHROMEOS_EXPORT extern const char kJemaOSWallpapersBasePath[];

// Path to the backup script
CHROMEOS_EXPORT extern const char kJemaOSBackupScriptPath[];

// Path to the restore script
CHROMEOS_EXPORT extern const char kJemaOSRestoreScriptPath[];

}  // namespace constants
}  // namespace jemaos

#endif  // CHROMEOS_JEMAOS_SWITCHES_MISC_CONSTANTS_H_