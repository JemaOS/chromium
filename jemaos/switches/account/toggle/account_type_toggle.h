// Copyright (c) 2022 Jema Technology. All rights reserved.
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

CHROMEOS_EXPORT void EnableJemaAccountFlag();
CHROMEOS_EXPORT void DisableJemaAccountFlag();
CHROMEOS_EXPORT void ToggleJemaAccountFlagByAccountId(const AccountId& account_id);
CHROMEOS_EXPORT void ToggleJemaAccountFlagForCommandLine(base::CommandLine* command_line);
CHROMEOS_EXPORT void ToggleJemaAccountFlagForCommandLineByAccountId(base::CommandLine* command_line, const AccountId& account_id);
CHROMEOS_EXPORT void ToggleJemaAccountFlagByActiveUser();
CHROMEOS_EXPORT void EnableJemaAccountFlagForManagedDevice();
CHROMEOS_EXPORT void DisableJemaAccountFlagForManagedDevice();
CHROMEOS_EXPORT void AppendAccountSwitchesIfNeed(const AccountId& account_id, std::vector<std::string>* switches);

} // namespace switches
} // namespace jemaos


#endif  // JEMAOS_SWITCHES_ACCOUNT_TOGGLE_ACCOUNT_TYPE_TOGGLE_H_
