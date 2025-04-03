// Copyright (c) 2022 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_SWITCHES_ACCOUNT_ACCOUNT_BASE_H_
#define JEMAOS_SWITCHES_ACCOUNT_ACCOUNT_BASE_H_

#include "chromeos/chromeos_export.h"

namespace jemaos {
namespace switches {

CHROMEOS_EXPORT void JemaSetDeviceManagedFlag(bool is_managed);
CHROMEOS_EXPORT bool IsJemaSetDeviceManaged();

} // namespace switches
} // namespace jemaos


#endif  // // Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_SWITCHES_ACCOUNT_ACCOUNT_BASE_H_
#define JEMAOS_SWITCHES_ACCOUNT_ACCOUNT_BASE_H_

#include "chromeos/chromeos_export.h"

namespace jemaos {
namespace switches {

// Sets the device managed flag
CHROMEOS_EXPORT void JemaSetDeviceManagedFlag(bool is_managed);

// Checks if the device is managed
CHROMEOS_EXPORT bool IsJemaSetDeviceManaged();

}  // namespace switches
}  // namespace jemaos

#endif  // JEMAOS_SWITCHES_ACCOUNT_ACCOUNT_BASE_H_
