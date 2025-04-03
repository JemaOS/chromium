// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JEMAOS_EXTENSIONS_COMMON_JEMAOS_API_PERMISSIONS_H_
#define JEMAOS_EXTENSIONS_COMMON_JEMAOS_API_PERMISSIONS_H_

#include "base/containers/span.h"
#include "extensions/common/alias.h"
#include "extensions/common/permissions/api_permission.h"

namespace extensions {

// Registers the permissions used in JemaOS with the PermissionsInfo global.
namespace jemaos_api_permissions {

  // Returns the list of permission infos
  base::span<const APIPermissionInfo::InitInfo> GetPermissionInfos();

  // Returns the list of permission aliases
  base::span<const Alias> GetPermissionAliases();

}  // namespace jemaos_api_permissions

}  // namespace extensions

#endif  // JEMAOS_EXTENSIONS_COMMON_JEMAOS_API_PERMISSIONS_H_