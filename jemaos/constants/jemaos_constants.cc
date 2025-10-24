// Copyright 2024 The Jema Technology Limited. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/constants/jemaos_constants.h"

namespace jemaos::constants {

namespace {

const char kJemaSystemControllerExtensionId[] = "mofiofjpikncjaigmdlblhojbnkabako";
const char kJemaRdpExtensionId[] = "fogdcaodknbhigpklbhepedofamkfbln";

constexpr char const* kExtensionIdsHiddenInList[] = {
    kJemaSystemControllerExtensionId,
    kJemaRdpExtensionId,
};

}

bool ShouldHideExtensionById(const std::string& extension_id) {
  for (auto* const id : kExtensionIdsHiddenInList) {
    if (id == extension_id) {
      return true;
    }
  }
  return false;
}

} // namespace jemaos::constants
