// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/switches/account/base/account_base.h"

namespace jemaos {
namespace switches {

namespace {

// Flag indicating whether the device is managed
bool is_device_managed = false;

}  // namespace

// Sets the device managed flag
void JemaSetDeviceManagedFlag(bool is_managed) {
  is_device_managed = is_managed;
}

// Checks if the device is managed
bool IsJemaSetDeviceManaged() {
  return is_device_managed;
}

}  // namespace switches
}  // namespace jemaos