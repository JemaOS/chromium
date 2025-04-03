// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_SWITCHES_ACCOUNT_TOGGLE_ACCOUNT_TYPE_TOGGLE_H_
#define JEMAOS_SWITCHES_ACCOUNT_TOGGLE_ACCOUNT_TYPE_TOGGLE_H_

#include "chromeos/chromeos_export.h"
#include <string>
#include <vector>

namespace base {
  class CommandLine;
}
class AccountId;

namespace jemaos {
namespace switches {

// Enables the Jema account flag
CHROMEOS_EXPORT void EnableJemaAccountFlag();

// Disables the Jema account flag
CHROMEOS_EXPORT void DisableJemaAccountFlag();

// Toggles the Jema account flag by account ID
CHROMEOS_EXPORT void ToggleJemaAccountFlagByAccountId(const AccountId& account_id);

// Toggles the Jema account flag for the current command line
CHROMEOS_EXPORT void ToggleJemaAccountFlagForCommandLine(base::CommandLine* command_line);

// Toggles the Jema account flag for the command line by account ID
CHROMEOS_EXPORT void ToggleJemaAccountFlagForCommandLineByAccountId(base::CommandLine* command_line, const AccountId& account_id);

// Toggles the Jema account flag by the active user
CHROMEOS_EXPORT void ToggleJemaAccountFlagByActiveUser();

// Enables the Jema account flag for a managed device
CHROMEOS_EXPORT void EnableJemaAccountFlagForManagedDevice();

// Disables the Jema account flag for a managed device
CHROMEOS_EXPORT void DisableJemaAccountFlagForManagedDevice();

// Appends account switches if needed
CHROMEOS_EXPORT void AppendAccountSwitchesIfNeed(const AccountId& account_id, std::vector<std::string>* switches);

}  // namespace switches
}  // namespace jemaos

#endif  // JEMAOS_SWITCHES_ACCOUNT_TOGGLE_ACCOUNT_TYPE_TOGGLE_H_