// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_JEMAOS_SWITCHES_ACCOUNT_POLICY_CONSTANTS_H_
#define CHROMEOS_JEMAOS_SWITCHES_ACCOUNT_POLICY_CONSTANTS_H_

#include <string>
#include "chromeos/chromeos_export.h"

namespace jemaos {
namespace constants {

// Policy verification key for JemaOS
CHROMEOS_EXPORT extern const uint8_t kJemaOSPolicyVerificationKey[];
extern const size_t kJemaOSPolicyVerificationKeyLength;

// Sender ID for JemaOS policy FCM invalidation
CHROMEOS_EXPORT extern const char kJemaOSPolicyFCMInvalidationSenderID[];

}  // namespace constants
}  // namespace jemaos

#endif  // CHROMEOS_JEMAOS_SWITCHES_ACCOUNT_POLICY_CONSTANTS_H_