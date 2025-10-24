// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "jemaos/extensions/common/jemaos_extensions_api_provider.h"

#include "jemaos/extensions/common/api/api_features.h"
#include "jemaos/extensions/common/api/generated_schemas.h"
#include "jemaos/extensions/common/api/permission_features.h"
#include "jemaos/extensions/common/jemaos_api_permissions.h"
#include "jemaos/extensions/common/grit/jemaos_extensions_resources.h"
#include "extensions/common/features/json_feature_provider_source.h"
#include "extensions/common/permissions/permissions_info.h"
#include "base/logging.h"

namespace extensions {

JemaOSExtensionsAPIProvider::JemaOSExtensionsAPIProvider() {}
JemaOSExtensionsAPIProvider::~JemaOSExtensionsAPIProvider() = default;

void JemaOSExtensionsAPIProvider::AddAPIFeatures(FeatureProvider* provider) {
  AddJemaOSAPIFeatures(provider);
  VLOG(1) << "Add jemaos api features\n";
}

void JemaOSExtensionsAPIProvider::AddManifestFeatures(
    FeatureProvider* provider) {
}

void JemaOSExtensionsAPIProvider::AddPermissionFeatures(
    FeatureProvider* provider) {
  AddJemaOSPermissionFeatures(provider);
  VLOG(1) << "Add jemaos permission features\n";
}

void JemaOSExtensionsAPIProvider::AddBehaviorFeatures(
    FeatureProvider* provider) {
  // Note: No chrome-specific behavior features.
}

void JemaOSExtensionsAPIProvider::AddAPIJSONSources(
    JSONFeatureProviderSource* json_source) {
  json_source->LoadJSON(IDR_JEMAOS_EXTENSION_API_FEATURES);
}

bool JemaOSExtensionsAPIProvider::IsAPISchemaGenerated(
    const std::string& name) {
  return api::JemaOSGeneratedSchemas::IsGenerated(name);
}

std::string_view JemaOSExtensionsAPIProvider::GetAPISchema(
    const std::string& name) {
  return api::JemaOSGeneratedSchemas::Get(name);
}

void JemaOSExtensionsAPIProvider::RegisterPermissions(PermissionsInfo* permissions_info) {
  permissions_info->RegisterPermissions(
      jemaos_api_permissions::GetPermissionInfos(),
      jemaos_api_permissions::GetPermissionAliases());
  VLOG(1) << "register jemaos permissions";
}

void JemaOSExtensionsAPIProvider::RegisterManifestHandlers() {
}

}  // namespace extensions
