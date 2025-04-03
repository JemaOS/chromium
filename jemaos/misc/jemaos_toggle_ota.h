// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_TOGGLE_OTA_H_
#define JEMAOS_TOGGLE_OTA_H_

#include "base/functional/callback_forward.h"

namespace jemaos {
namespace misc {

  // Enables or disables JemaOS OTA updates
  void EnableJemaOTA(const bool enabled, base::OnceCallback<void()> callback);

  // Checks if JemaOS OTA updates are enabled
  bool GetEnabledJemaOTA();

}  // namespace misc
}  // namespace jemaos

#endif  // JEMAOS_TOGGLE_OTA_H_