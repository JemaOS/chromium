// Copyright (c) 2022 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/account/base/account_base.h"

namespace jemaos {
namespace switches {

namespace {
bool is_device_managed = false;
}

void JemaSetDeviceManagedFlag(bool is_managed) {
  is_device_managed = is_managed;
}

bool IsJemaSetDeviceManaged() {
  return is_device_managed;
}

} // namespace switches
} // namespace jemaos
