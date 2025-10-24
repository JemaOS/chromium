// Copyright 2020 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_CALLBACK_STATUS_H_
#define JEMAOS_CALLBACK_STATUS_H_

#include <string>

#include "base/functional/callback.h"
#include "base/component_export.h"

namespace jemaos::license {
template <typename ResultType>
  using SuccessCallback = base::OnceCallback<void(ResultType result)>;
  using SavePrefCallback = base::OnceCallback<void(
    int licenseType, bool expired, int expiration_action, int showLicenseInSettings, int logOutInterval)>;
  using ErrorCallback =
    base::OnceCallback<void(int errCode, const std::string& errMsg)>;
template <typename ResultType>
  using ErrorWithSaveCallback =
    base::OnceCallback<void(int errCode, const std::string& errMsg, ResultType result)>;

}  // namespace jemaos::license

#endif  // JEMAOS_CALLBACK_STATUS_H_
