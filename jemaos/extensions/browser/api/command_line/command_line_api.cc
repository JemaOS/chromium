// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/extensions/browser/api/command_line/command_line_api.h"
#include "jemaos/extensions/common/api/command_line.h"

#include <memory>
#include <string>

#include "base/command_line.h"
#include "base/values.h"

namespace extensions {
  namespace command_line = api::command_line;
  const char kEmptySwitchName[] = "Switch name is empty.";

  namespace {
    // Checks if a switch exists
    bool HasSwitch(const std::string& name) {
      return base::CommandLine::ForCurrentProcess()->HasSwitch(name);
    }

    // Retrieves the value of a switch
    std::string GetSwitchValue(const std::string& name) {
      return base::CommandLine::ForCurrentProcess()->GetSwitchValueASCII(name);
    }
  }  // namespace

  // Handles the "HasSwitch" function
  ExtensionFunction::ResponseAction CommandLineHasSwitchFunction::Run() {
    absl::optional<command_line::HasSwitch::Params> params(
      command_line::HasSwitch::Params::Create(args()));
    EXTENSION_FUNCTION_VALIDATE(params);
    if (params->name.empty())
      return RespondNow(Error(kEmptySwitchName));
    if (!HasSwitch(params->name))
      return RespondNow(ArgumentList(command_line::HasSwitch::Results::Create(false, std::string())));
    return RespondNow(ArgumentList(command_line::HasSwitch::Results::Create(true, GetSwitchValue(params->name))));
  }

  // Handles the "AddSwitch" function
  ExtensionFunction::ResponseAction CommandLineAddSwitchFunction::Run() {
    absl::optional<command_line::AddSwitch::Params> params(
      command_line::AddSwitch::Params::Create(args()));
    EXTENSION_FUNCTION_VALIDATE(params);
    if (params->switch_info.name.empty())
      return RespondNow(Error(kEmptySwitchName));
    if (params->switch_info.value)
      base::CommandLine::ForCurrentProcess()->AppendSwitchASCII(params->switch_info.name, *params->switch_info.value);
    else
      base::CommandLine::ForCurrentProcess()->AppendSwitch(params->switch_info.name);
    return RespondNow(NoArguments());
  }

  // Handles the "RemoveSwitch" function
  ExtensionFunction::ResponseAction CommandLineRemoveSwitchFunction::Run() {
    absl::optional<command_line::RemoveSwitch::Params> params(
      command_line::RemoveSwitch::Params::Create(args()));
    EXTENSION_FUNCTION_VALIDATE(params);
    if (params->name.empty())
      return RespondNow(Error(kEmptySwitchName));
    base::CommandLine::ForCurrentProcess()->RemoveSwitch(params->name);
    return RespondNow(NoArguments());
  }

}  // namespace extensions