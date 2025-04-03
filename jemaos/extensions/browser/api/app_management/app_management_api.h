#ifndef JEMAOS_EXTENSIONS_API_APP_MANAGEMENT_API_H_
#define JEMAOS_EXTENSIONS_API_APP_MANAGEMENT_API_H_

#include <string>
#include <vector>
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"

namespace extensions {

  // Handles the "GetAppList" function
  class AppManagementGetAppListFunction: public ExtensionFunction {
    ~AppManagementGetAppListFunction() override {}
    ResponseAction Run() override;
    DECLARE_EXTENSION_FUNCTION("appManagement.getAppList", APP_MANAGEMENT_GETAPPLIST)
  };

  // Handles the "InstallWebApp" function
  class AppManagementInstallWebAppFunction: public ExtensionFunction {
    ~AppManagementInstallWebAppFunction() override {}
    ResponseAction Run() override;
    DECLARE_EXTENSION_FUNCTION("appManagement.installWebApp", APP_MANAGEMENT_INSTALLWEBAPP)
  };

  // Handles the "InstallArcApp" function
  class AppManagementInstallArcAppFunction: public ExtensionFunction {
    ~AppManagementInstallArcAppFunction() override {}
    ResponseAction Run() override;
    DECLARE_EXTENSION_FUNCTION("appManagement.installArcApp", APP_MANAGEMENT_INSTALLARCAPP)
  };

  // Handles the "GetArcPolicy" function
  class AppManagementGetArcPolicyFunction: public ExtensionFunction {
    ~AppManagementGetArcPolicyFunction() override {}
    ResponseAction Run() override;
    DECLARE_EXTENSION_FUNCTION("appManagement.getArcPolicy", APP_MANAGEMENT_GETARCPOLICY)
  };

}  // namespace extensions

#endif  // JEMAOS_EXTENSIONS_API_APP_MANAGEMENT_API_H_