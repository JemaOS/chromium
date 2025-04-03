// Copyright 2025 Jema Technology. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/extensions/common/jemaos_api_permissions.h"

#include <stddef.h>

#include <memory>

#include "extensions/common/permissions/api_permission.h"

namespace extensions {

namespace {

  // Template function to create an API permission
  template <typename T>
  std::unique_ptr<APIPermission> CreateAPIPermission(
      const APIPermissionInfo* permission) {
    return std::make_unique<T>(permission);
  }

  // List of permissions to register
  constexpr APIPermissionInfo::InitInfo permissions_to_register[] = {
      // Register permissions for all extension types.
      {mojom::APIPermissionID::kJemaOSPrivate, "JemaOSPrivate",
       APIPermissionInfo::kFlagCannotBeOptional},
      {mojom::APIPermissionID::kJemaOSShellClient, "JemaOSShellClient"}
  };

}  // namespace

namespace jemaos_api_permissions {

  // Returns the list of permission infos
  base::span<const APIPermissionInfo::InitInfo> GetPermissionInfos() {
    return base::make_span(permissions_to_register);
  }

  // Returns the list of permission aliases
  base::span<const Alias> GetPermissionAliases() {
    // In alias constructor, first value is the alias name; second value is the
    // real name. See also alias.h.
    return base::span<const extensions::Alias>();
  }

}  // namespace jemaos_api_permissions
}  // namespace extensions