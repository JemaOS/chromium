// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_EXTENSIONS_BROWSER_API_JEMAOS_PRIVATE_JEMAOS_PRIVATE_API_H_
#define JEMAOS_EXTENSIONS_BROWSER_API_JEMAOS_PRIVATE_JEMAOS_PRIVATE_API_H_

#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"

namespace extensions {

// Handles the "GetJemaOSInfo" function
class JemaosPrivateGetJemaOSInfoFunction : public ExtensionFunction {
 public:
  // Declares the extension function
  DECLARE_EXTENSION_FUNCTION("jemaosPrivate.getJemaOSInfo",
                              JEMAOS_PRIVATE_GET_JEMAOS_INFO)

 protected:
  // Destructor
  ~JemaosPrivateGetJemaOSInfoFunction() override = default;

  // Executes the function
  ResponseAction Run() override;
};

}  // namespace extensions

#endif  // JEMAOS_EXTENSIONS_BROWSER_API_JEMAOS_PRIVATE_JEMAOS_PRIVATE_API_H_