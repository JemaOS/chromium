// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/misc/misc_constants.h"

namespace jemaos {
namespace constants {

// Default time zone ID for JemaOS
const char kJemaOSDefaultTimeZoneId[] = "Europe/Paris";

// Header for the system information section
const char kJemaOSSystemInfoHeader[] = "--- Full System ---";

// File name for exporting system information
const char kJemaOSSystemInfoFileName[] = "about_system.txt";

// Temporary prefix for system information files
const char kJemaOSSystemTempPrefix[] = "jemaos_sysinfo";

// Command to retrieve hardware tuner information
const char kJemaOSHwtunerCommand[] = "/usr/share/hwtuner-script/hwtuner_info";

// Section name for DMI information in the hardware tuner output
const char kJemaOSHwtunerInfoSectionName[] = "--- DMI Info ---";

// Base path for JemaOS wallpapers
const char kJemaOSWallpapersBasePath[] = "/usr/share/chromeos-assets/jemaos_wallpapers/";

// Path to the backup script
const char kJemaOSBackupScriptPath[] = "/usr/bin/jemaos-backup";

// Path to the restore script
const char kJemaOSRestoreScriptPath[] = "/usr/bin/jemaos-backup";

}  // namespace constants
}  // namespace jemaos
