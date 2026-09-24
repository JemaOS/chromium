// Copyright 2026 jema technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_ASH_LOGIN_JEMA_PASSWORD_VALIDATOR_H_
#define CHROME_BROWSER_UI_ASH_LOGIN_JEMA_PASSWORD_VALIDATOR_H_

#include <string>

#include "base/functional/callback.h"

namespace ash {

// Silently validates a Jema account password against the Connect API
// (`osLogin`) from the login screen, where no Profile is available. Used to
// apply a SaaS password change transparently: the user keeps typing at the
// normal pod password field, and this validates the typed password online.
//
// The callback is always invoked exactly once, with `true` when the Connect
// API accepted the credentials (HTTP 200 "OS login successful"), `false`
// otherwise (wrong password, network error, missing data, ...).
class JemaPasswordValidator {
 public:
  JemaPasswordValidator() = delete;

  static void Validate(const std::string& email,
                       const std::string& password,
                       base::OnceCallback<void(bool)> callback);
};

}  // namespace ash

#endif  // CHROME_BROWSER_UI_ASH_LOGIN_JEMA_PASSWORD_VALIDATOR_H_
