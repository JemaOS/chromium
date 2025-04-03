#ifndef JEMAOS_EXTENSIONS_API_NATIVE_WINDOWS_NATIVE_WINDOWS_API_H_
#define JEMAOS_EXTENSIONS_API_NATIVE_WINDOWS_NATIVE_WINDOWS_API_H_

#include <string>
#include <vector>
#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"

namespace extensions {

// Handles the "Get" function for native windows
class NativeWindowsGetFunction : public ExtensionFunction {
  ~NativeWindowsGetFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nativeWindows.get", NATIVE_WINDOWS_GET)
};

// Handles the "GetAll" function for native windows
class NativeWindowsGetAllFunction : public ExtensionFunction {
  ~NativeWindowsGetAllFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nativeWindows.getAll", NATIVE_WINDOWS_GETALL)
};

// Handles the "Update" function for native windows
class NativeWindowsUpdateFunction : public ExtensionFunction {
  ~NativeWindowsUpdateFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nativeWindows.update", NATIVE_WINDOWS_UPDATE)
};

// Handles the "Remove" function for native windows
class NativeWindowsRemoveFunction : public ExtensionFunction {
  ~NativeWindowsRemoveFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nativeWindows.remove", NATIVE_WINDOWS_REMOVE)
};

// Handles the "Create" function for native windows
class NativeWindowsCreateFunction : public ExtensionFunction {
  ~NativeWindowsCreateFunction() override {}
  ResponseAction Run() override;
  DECLARE_EXTENSION_FUNCTION("nativeWindows.create", NATIVE_WINDOWS_CREATE)
};

}  // namespace extensions

#endif  // JEMAOS_EXTENSIONS_API_NATIVE_WINDOWS_NATIVE_WINDOWS_API_H_