#ifndef JEMAOS_EXTENSIONS_API_COMMAND_LINE_API_H_
#define JEMAOS_EXTENSIONS_API_COMMAND_LINE_API_H_

#include <string>
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"

namespace extensions {

  // Handles the "HasSwitch" function
  class CommandLineHasSwitchFunction: public ExtensionFunction {
    ~CommandLineHasSwitchFunction() override {}
    ResponseAction Run() override;
    DECLARE_EXTENSION_FUNCTION("commandLine.hasSwitch", COMMAND_LINE_HASSWITCH)
  };

  // Handles the "RemoveSwitch" function
  class CommandLineRemoveSwitchFunction: public ExtensionFunction {
    ~CommandLineRemoveSwitchFunction() override {}
    ResponseAction Run() override;
    DECLARE_EXTENSION_FUNCTION("commandLine.removeSwitch", COMMAND_LINE_REMOVESWITCH)
  };

  // Handles the "AddSwitch" function
  class CommandLineAddSwitchFunction: public ExtensionFunction {
    ~CommandLineAddSwitchFunction() override {}
    ResponseAction Run() override;
    DECLARE_EXTENSION_FUNCTION("commandLine.addSwitch", COMMAND_LINE_ADDSWITCH)
  };

}  // namespace extensions

#endif  // JEMAOS_EXTENSIONS_API_COMMAND_LINE_API_H_