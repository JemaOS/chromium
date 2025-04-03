// Copyright 2025 Jema Technology. All rights reserved.
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

// Constructor for JemaOSExtensionsAPIProvider
JemaOSExtensionsAPIProvider::JemaOSExtensionsAPIProvider() {}

// Destructor for JemaOSExtensionsAPIProvider
JemaOSExtensionsAPIProvider::~JemaOSExtensionsAPIProvider() = default;

// Adds API features to the feature provider
void JemaOSExtensionsAPIProvider::AddAPIFeatures(FeatureProvider* provider) {
  AddJemaOSAPIFeatures(provider);
  VLOG(1) << "Add jemaos api features\n";
}

// Adds manifest features to the feature provider
void JemaOSExtensionsAPIProvider::AddManifestFeatures(
    FeatureProvider* provider) {
  // No manifest features to add for JemaOS.
}

// Adds permission features to the feature provider
void JemaOSExtensionsAPIProvider::AddPermissionFeatures(
    FeatureProvider* provider) {
  AddJemaOSPermissionFeatures(provider);
  VLOG(1) << "Add jemaos permission features\n";
}

// Adds behavior features to the feature provider
void JemaOSExtensionsAPIProvider::AddBehaviorFeatures(
    FeatureProvider* provider) {
  // Note: No JemaOS-specific behavior features.
}

// Adds API JSON sources to the JSON feature provider
void JemaOSExtensionsAPIProvider::AddAPIJSONSources(
    JSONFeatureProviderSource* json_source) {
  json_source->LoadJSON(IDR_JEMAOS_EXTENSION_API_FEATURES);
}

// Checks if an API schema is generated
bool JemaOSExtensionsAPIProvider::IsAPISchemaGenerated(
    const std::string& name) {
  return api::JemaOSGeneratedSchemas::IsGenerated(name);
}

// Retrieves the API schema for a given name
base::StringPiece JemaOSExtensionsAPIProvider::GetAPISchema(
    const std::string& name) {
  return api::JemaOSGeneratedSchemas::Get(name);
}

// Registers permissions with the PermissionsInfo global
void JemaOSExtensionsAPIProvider::RegisterPermissions(
    PermissionsInfo* permissions_info) {
  permissions_info->RegisterPermissions(
      jemaos_api_permissions::GetPermissionInfos(),
      jemaos_api_permissions::GetPermissionAliases());
  VLOG(1) << "register jemaos permissions";
}

// Registers manifest handlers
void JemaOSExtensionsAPIProvider::RegisterManifestHandlers() {
  // No manifest handlers to register for JemaOS.
}

}  // namespace extensions