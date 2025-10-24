// Copyright (c) 2019 The JemaOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#ifndef CHROMEOS_JEMAOS_SWITCHES_ACCOUNT_POLICY_CONSTANTS_H_
#define CHROMEOS_JEMAOS_SWITCHES_ACCOUNT_POLICY_CONSTANTS_H_

#include <string>
#include "chromeos/chromeos_export.h"

namespace jemaos {
namespace constants {

CHROMEOS_EXPORT extern const uint8_t kJemaOSPolicyVerificationKey[];
extern const size_t kJemaOSPolicyVerificationKeyLength;

inline constexpr char kJemaOSPolicyFCMInvalidationSenderID[] = "384261808202";

extern const char kJemaOSOobeZteConfigFile[];
}
}

#endif
