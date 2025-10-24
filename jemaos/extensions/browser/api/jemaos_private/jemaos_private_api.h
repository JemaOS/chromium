// Copyright 2023 The Jema Technology Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef JEMAOS_EXTENSIONS_BROWSER_API_JEMAOS_PRIVATE_JEMAOS_PRIVATE_API_H_
#define JEMAOS_EXTENSIONS_BROWSER_API_JEMAOS_PRIVATE_JEMAOS_PRIVATE_API_H_

#include "extensions/browser/extension_function.h"
#include "extensions/browser/extension_function_histogram_value.h"

namespace extensions {
class JemaosPrivateGetJemaOSInfoFunction: public ExtensionFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("jemaosPrivate.getJemaOSInfo",
                              JEMAOS_PRIVATE_GET_JEMAOS_INFO)

 protected:
  ~JemaosPrivateGetJemaOSInfoFunction() override = default;
  ResponseAction Run() override;
};
}  // namespace extensions


#endif  // JEMAOS_EXTENSIONS_BROWSER_API_JEMAOS_PRIVATE_JEMAOS_PRIVATE_API_H_
